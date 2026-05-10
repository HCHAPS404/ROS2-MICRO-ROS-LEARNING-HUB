from setuptools import find_packages, setup

package_name = "range_visualizer"

setup(
    name=package_name,
    version="0.1.0",
    packages=find_packages(exclude=["test"]),
    data_files=[
        ("share/ament_index/resource_index/packages", ["resource/" + package_name]),
        ("share/" + package_name, ["package.xml"]),
    ],
    install_requires=["setuptools"],
    zip_safe=True,
    maintainer="maintainer",
    maintainer_email="user@example.com",
    description="Visualización para /ultrasonic/range (sensor_msgs/Range).",
    license="Apache-2.0",
    tests_require=["pytest"],
    entry_points={
        "console_scripts": [
            "range_visualizer = range_visualizer.range_visualizer_node:main",
        ],
    },
)
