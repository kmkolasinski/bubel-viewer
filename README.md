# Bubel-Viewer 2020-12-11

A simple 3D visualization tool for tight binding models

![image](img/header-image.png)

# Data format

## Representing Tight-Bingding system

Viewer can read XML file which has following structure

## Displaying onsite 


# Building the Source Code

## Requirements

The application has been tested on linux Ubuntu 18 LTS system, however since it 
is purely written in Qt a crossplatform it should also work on other systems.

Program has been compiled with:
* Qt 5.15.2 GCC 64bit
* Additional OpenGL related libraries may be required for building, for linux see e.g. this [thread](https://forum.qt.io/topic/50865/gl-gl-h-not-found-in-linux/10)

## Builing steps 

* The application must be compiled from source 
* Download and install QtCreator - https://www.qt.io/download (follow "Downloads for open source users")
* Start QtCreator application
* Select **Open Project** and navigate to **bubel-viewer/src/viewer.pro** file.

![image](img/building-instructions-01.png)

* Select supported by your system compiler and press **Configure Project**

![image](img/building-instructions-02.png)

* Build Project (optionally switch to the Release mode for greater performance) by pressing **hammer button** in the bottom-left corner.

![image](img/building-instructions-03.png)

* After compilation Run application, by clicking green **play button** in the bottom-left corner. 
* After this step, you can find binary file **viewer** or **viewer.exe** in the **bubel/viewer/build-viewer-.../**  folder. 
You should be able to run viewer by double clicking on this icon. 



