import setuptools

DESCRIPTION = """\
Wicope is a fast oscilloscope GUI application that makes use of
Arduino to acquire signals.

This is not a professional oscilloscope, but it can be useful
for electronics enthusiasts and for education.
"""

with open("requirements_users.txt") as f:
    required_packages = f.read().splitlines()

setuptools.setup(
    name="wicope",
    version="0.1.0b1",
    author="Diego Palacios",
    author_email="diepala@gmail.com",
    description="A fast Arduino Oscilloscope",
    long_description=DESCRIPTION,
    url="https://github.com/diepala/wicope",
    packages=setuptools.find_packages(),
    classifiers=[
        "Programming Language :: Python :: 3",
        "Operating System :: OS Independent",
        "Intended Audience :: Education",
        "Intended Audience :: Science/Research",
        "Topic :: Education",
        "Topic :: Scientific/Engineering",
    ],
    python_requires=">=3.7",
    install_requires=required_packages,
    entry_points={"console_scripts": ["wicope = wicope.app:main"]},
)
