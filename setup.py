import setuptools

with open("README.md", "r") as fh:
    long_description = fh.read()

setuptools.setup(
    name="python-alea",
    version="0.0.2",
    author="Edwin Dalmaijer",
    author_email="edwin.dalmaijer@gmail.com",
    description="Python wrapper for the IntelliGaze API by Alea Technologies",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://github.com/esdalmaijer/PyAlea",
    packages=setuptools.find_packages(),
    data_files=[("python-alea",["./python-alea/EtApi.dll","./python-alea/CEtAPI.dll","./python-alea/CEtAPIx64.dll"])],
    classifiers=[
        "Programming Language :: Python :: 2",
        "License :: OSI Approved :: Apache Software License",
        "Operating System :: Microsoft :: Windows",
    ],
)
