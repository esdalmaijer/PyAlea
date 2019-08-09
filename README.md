# PyAlea

Python wrapper for the IntelliGaze API by [Alea Technologies](https://www.intelligaze.com/en/).

## C API

Alea offers an API to interact with their eye trackers. This API operates through COM, and is less straightforward to wrap in Python. (While technically possible, it requires external packages, and jumping through several hoops.) An easier solution was to introduce a C++ wrapper for the API, which is then compiled as a DLL. The resulting DLL is more easily accessed through Python's `ctypes`.

The source code for this "C API" wrapper for the original API is included here.

## Download and install

PyAlea is available via the Python Package Index (PyPI), and can be installed through `pip`.

```
pip install python-alea
```


