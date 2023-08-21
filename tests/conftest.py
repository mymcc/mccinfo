import pytest
import os

@pytest.fixture(scope="session", autouse=False)
def root():
    root: str = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    return root

@pytest.fixture(scope="session", autouse=True)
def setup(root):
    yield
