import alea
import setuptools

with open("README.md", "r") as fh:
    long_description = fh.read()
# Stupid bug in twine ignores long_description_content_type, so just refer
# people to GitHub.
long_description = "A bug in twine prevents reading Markdown files, so please read the README at https://github.com/esdalmaijer/PyAlea"

setuptools.setup(
    name="python-alea",
    version=alea.__version__,
    author="Edwin Dalmaijer",
    author_email="edwin.dalmaijer@gmail.com",
    description="Python wrapper for the IntelliGaze API by Alea Technologies",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://github.com/esdalmaijer/PyAlea",
    packages=["alea"],
    include_package_data=True,
    package_data={"alea":["*.dll"]},
    classifiers=[
        "Intended Audience :: Science/Research",
        "Topic :: Scientific/Engineering",
        "Programming Language :: Python :: 2",
        "License :: OSI Approved :: Apache Software License",
        "Operating System :: Microsoft :: Windows",
    ],
)
