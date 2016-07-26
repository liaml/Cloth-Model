# Cloth Model
Final Year Project, The University of Hull

THIS CODE SUPPORTS THE 08341 FYDP THESIS, (LEES, 2016), 'CLOTH MODELLING', SUBMITTED IN PARTIAL FULFILMENT OF THE REQUIREMENTS FOR THE DEGREE OF 'BACHELOR OF SCIENCE' IN THE FACULTY OF UNDERGRADUATE STUDIES (DEPT. OF COMPUTER SCIENCE, THE UNIVERSITY OF HULL)

## Features
* Physical Mass-Sping Force-Based Constraints System
* Simple ATB interface
* Static Collisions 
* Self-Colisions :
  - Ray-Triangle intersection method (not very effective)
* Wind
* Integration methods :
  - Explicit Forward Euler
  - Runge-Kutta (RK2 & RK4)
  - Symplectic Euler
  - IMplicit-EXplicit IMEX solution
  - Implicit Backward Euler :
    - Using a Congugate Gradient Solver
    - Using a Gradient Descent Solver
  - Position Based Verlet

## Dependancies
- GLEW
- GLFW
- GLM
- AntTweakBar

## Screenshot
![Cloth demo](http://pasteboard.co/images/6TxveCRo.tiff/download "Cloth demo")
