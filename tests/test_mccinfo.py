import pytest

@pytest.mark.parametrize("mcc_version",
                         [
                             ("Steam"),
                             ("Windows Store"),
                         ]
                         )
def test_mccinfo(root, mcc_version):
    assert(1 == 1)