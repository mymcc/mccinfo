#pragma once

namespace mccinfo {
namespace fsm {

namespace details {
inline void flatten(
    const std::filesystem::path & current_root_path, 
    const std::filesystem::path & target_root_path) {
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = INVALID_HANDLE_VALUE;

    const std::filesystem::path &search_pattern =
        std::filesystem::absolute(current_root_path) / "*";

    // Find the first file in the directory.
    hFind = FindFirstFile(search_pattern.generic_wstring().c_str(), &findFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        //MessageBox(NULL, search_pattern.generic_wstring().c_str(),
        //           current_root_path.generic_wstring().c_str(), MB_OK);
        std::wcerr << L"FindFirstFile failed for " << search_pattern.generic_wstring()
                   << L" with error " << GetLastError()
                   << std::endl;
        return;
    }

    do {
        // Skip '.' and '..' directories
        if (wcscmp(findFileData.cFileName, L".") == 0 ||
            wcscmp(findFileData.cFileName, L"..") == 0) {
            continue;
        }

        const std::filesystem::path& file_path = 
            current_root_path / std::wstring(findFileData.cFileName);

        // Check if found entity is a directory
        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            // Recursively flatten the found directory
            flatten(file_path, target_root_path);
            // Attempt to remove the now-empty directory
            if (RemoveDirectory(file_path.generic_wstring().c_str()) == 0) {
                std::wstring err_msg = L"RemoveDirectory failed trying to delete: " +
                                       current_root_path.generic_wstring() +
                                       L" with error: " + std::to_wstring(GetLastError());
                //MessageBox(NULL, err_msg.c_str(), current_root_path.generic_wstring().c_str(), MB_OK);
            }
        } else {
            // Construct new file path in the root directory
            const std::filesystem::path& new_file_path = target_root_path.generic_wstring() + L"\\" + findFileData.cFileName;

            std::wstring error_msg = L"Failed to move " + file_path.generic_wstring() + L" to " +
                                     new_file_path.generic_wstring() +
                                     L" with error " + std::to_wstring(GetLastError());
            //MessageBox(NULL, new_file_path.generic_wstring().c_str(), current_root_path.generic_wstring().c_str(), MB_OK);

            // only copy and delete if we aren't in targetRootPath
            if (current_root_path != target_root_path) {

                if (CopyFile(file_path.generic_wstring().c_str(), new_file_path.generic_wstring().c_str(), FALSE) == 0) {
                    // Handle error or file name conflicts
                    //MessageBox(NULL, error_msg.c_str(), current_root_path.generic_wstring().c_str(), MB_OK);

                    std::wcerr << L"Failed to move " << file_path.generic_wstring() << L" to "
                               << new_file_path.generic_wstring()
                               << L" with error " << GetLastError() << std::endl;
                }
                if (DeleteFile(file_path.generic_wstring().c_str()) == 0) {
                    std::wstring error_msg_2 = L"Failed to delete " +
                                               std::wstring(findFileData.cFileName) +
                                               L" with error " + std::to_wstring(GetLastError());
                    //MessageBox(NULL, new_file_path.generic_wstring().c_str(), current_root_path.generic_wstring().c_str(),
                    //           MB_OK);
                }
            }
        }
    } while (FindNextFile(hFind, &findFileData) != 0);

    FindClose(hFind);
}
} // namespace details

class autosave_client {
public:
    autosave_client() 
    {}

    autosave_client(
        const std::filesystem::path& src,
        const std::filesystem::path& dst,
        const std::filesystem::path& host) : 
        src_(src), dst_(dst), host_(host)
    {}

    void set_copy_src(const std::filesystem::path &new_src) {
        std::unique_lock<std::mutex> lock(mut_);
        src_ = new_src;
    }

    void set_copy_dst(const std::filesystem::path &new_dst) {
        std::unique_lock<std::mutex> lock(mut_);
        dst_ = new_dst;
    }

    void set_on_copy_start(std::function<void(const std::filesystem::path &)> pre_callback) {
        std::unique_lock<std::mutex> lock(mut_);
        pre_callback_ = pre_callback;
    }

    void set_on_complete(
        std::function<void(const std::filesystem::path &, const std::filesystem::path &)>
            post_callback) {
        std::unique_lock<std::mutex> lock(mut_);
        post_callback_ = post_callback;
    }

    void set_on_error(std::function<void(DWORD)> error_callback) {
        std::unique_lock<std::mutex> lock(mut_);
        error_callback_ = error_callback;
    }

    void set_flatten_on_write(bool flatten) {
        flatten_on_write_ = flatten;
    }

    void start() {
        copy_thread_ = std::thread([&] { 
            std::unique_lock<std::mutex> lock(mut_);

            while (true) {
                MI_CORE_TRACE("autosave_client waiting for request ...");

                cv_.wait(lock, [&] { return start_copy_ || stop_; });

                if (start_copy_)
                    MI_CORE_TRACE("autosave_client received request to copy from src: {0}",
                                  std::filesystem::absolute(src_).generic_string().c_str());
                if (stop_) {
                    MI_CORE_TRACE("autosave_client stopping ...");
                    break;
                }

                if (copy_delay_ms_ > 0) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(copy_delay_ms_));
                }

                create_dst_if_needed();

                if (pre_callback_) {
                    pre_callback_(dst_);
                }

                bool success = do_copy();

                if (success && post_callback_) {
                    post_callback_(src_, dst_);
                }

                if (flatten_on_write_) {
                    details::flatten(dst_, dst_);
                }

                else if (error_callback_){
                    error_callback_(GetLastError());
                }

                start_copy_ = false;
            }
        });

    }

    void stop() {
        std::unique_lock<std::mutex> lock(mut_);
        stop_ = true;
        cv_.notify_one();
    }

    void request_copy(uint32_t delay_ms = 0) {
        std::unique_lock<std::mutex> lock(mut_);
        start_copy_ = true;
        copy_delay_ms_ = delay_ms;
        cv_.notify_one();
    }

private:
    void create_dst_if_needed() {
        if (!std::filesystem::exists(dst_)) {
            MI_CORE_WARN(".\\mccinfo_cache\\autosave does not exist, creating: {0}",
                         dst_.make_preferred().generic_string().c_str());
            try {
                std::filesystem::create_directories(dst_);
            } catch (std::exception &e) {
                MI_CORE_ERROR("Creation of {0} failed.\nLastError: {1}\nException: {2}",
                    std::filesystem::absolute(dst_).generic_string().c_str(),
                    GetLastError(),
                    e.what()
                );
            }
        }
    }

    bool do_copy() {
        MI_CORE_TRACE("autosave_client starting copy of autosave cache: {0}", std::filesystem::absolute(src_).generic_string().c_str());

        STARTUPINFOW si;
        PROCESS_INFORMATION pi;

        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));

        wchar_t args[1024];
        wsprintf(args, L"%s -r -o %s -f %s",
            host_.generic_wstring().c_str(),
            dst_.generic_wstring().c_str(),
            src_.generic_wstring().c_str());

        if (CreateProcessW(host_.generic_wstring().c_str(), args, NULL, NULL, FALSE,
                           CREATE_NO_WINDOW, NULL,
                           NULL, &si,
                           &pi)) {
            WaitForSingleObject(pi.hProcess, INFINITE);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
            return true;
        } else {
            return false;
        }
    }


private:
    std::filesystem::path src_;
    std::filesystem::path dst_;
    std::filesystem::path host_;
    std::function<void(const std::filesystem::path &)> pre_callback_;
    std::function<void(const std::filesystem::path &, const std::filesystem::path &)> post_callback_;
    std::function<void(DWORD)> error_callback_;
    bool flatten_on_write_ = false;

  private:
    std::mutex mut_;
    std::condition_variable cv_;
    std::thread copy_thread_;
    bool start_copy_ = false;
    bool stop_ = false;
    uint32_t copy_delay_ms_ = 0;
};

}
}