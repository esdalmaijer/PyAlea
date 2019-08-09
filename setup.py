import setuptools

with open("README.md", "r") as fh:
    long_description = fh.read()

setuptools.setup(
    name="python-alea",
    version="0.0.1",
    author="Edwin Dalmaijer",
    author_email="edwin.dalmaijer@gmail.com",
    description="Python wrapper for the IntelliGaze API by Alea Technologies",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://github.com/esdalmaijer/PyAlea",
    packages=setuptools.find_packages(),
    classifiers=[
        "Programming Language :: Python :: 2",
        "License :: OSI Approved :: Apache License 2.0",
        "Operating System :: OS Independent",
    ],
)
