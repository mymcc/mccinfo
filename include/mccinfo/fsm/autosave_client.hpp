#pragma once

namespace mccinfo {
namespace fsm {

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

    void set_on_complete(std::function<void(const std::filesystem::path &)> post_callback) {
        std::unique_lock<std::mutex> lock(mut_);
        post_callback_ = post_callback;
    }

    void set_on_error(std::function<void(DWORD)> error_callback) {
        std::unique_lock<std::mutex> lock(mut_);
        error_callback_ = error_callback;
    }

    void start() {
        copy_thread_ = std::thread([&] { 
            std::unique_lock<std::mutex> lock(mut_);

            while (true) {
                cv_.wait(lock, [&] { return start_copy_ || stop_; });

                if (stop_)
                    break;

                if (copy_delay_ms_ > 0) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(copy_delay_ms_));
                }

                if (pre_callback_) {
                    pre_callback_(dst_);
                }

                bool success = do_copy();

                if (success && post_callback_) {
                    post_callback_(dst_);
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
    bool do_copy() {
        STARTUPINFOW si;
        PROCESS_INFORMATION pi;

        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));

        wchar_t args[1024];
        wsprintf(args, L"%s %s %s",
            host_.generic_wstring().c_str(),
            src_.generic_wstring().c_str(),
            dst_.generic_wstring().c_str());

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
    std::function<void(const std::filesystem::path &)> post_callback_;
    std::function<void(DWORD)> error_callback_;

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