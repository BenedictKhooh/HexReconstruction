# **Hexahedral Mesh Reconstruction from Point Cloud**

## **Overview**

This is a C++/Qt demonstration application that implements an algorithm to reconstruct a 3D hexahedral mesh structure from a given point cloud. The key challenge is to deduce the mesh's topology (how points are connected to form cells) purely from the points' spatial locations and predefined connectivity constraints.

The application provides a 3D visualizer to show the reconstruction process step-by-step, helping to understand the underlying geometric and topological logic.

## **Features**

* **3D Visualization**: Renders the input points, the calculated adjacency graph, the identified faces, and the final reconstructed hexahedra using OpenGL.  
* **Interactive Camera**: The 3D view can be manipulated with the mouse:  
  * **Rotate**: Hold the left mouse button and drag.  
  * **Zoom**: Use the mouse scroll wheel.  
* **Step-by-Step Execution**: The reconstruction process is broken down into three distinct, user-triggered steps to clearly visualize the algorithm's progression.  
* **Constrained Point Input**: The core algorithm relies on a MeshPoint structure, where each point is defined not just by its position but also by its expected number of neighbors, providing a robust foundation for the graph construction.

## **How to Build and Run**

This project is built using the Qt framework.

1. **Prerequisites**:  
   * A working installation of **Qt** (version 5.12 or newer is recommended).  
   * A C++ compiler configured with Qt Creator (e.g., MinGW, MSVC, Clang).  
2. **Setup in Qt Creator**:  
   * Create a new **Qt Widgets Application** project.  
   * Copy the contents of all the files (.pro, .h, .cpp) from the hexahedral-reconstruction-visual-app-constrained artifact into the corresponding files created by Qt Creator.  
   * Ensure your project's .pro file contains the necessary modules and libraries:  
     QT       \+= core gui widgets opengl  
     CONFIG   \+= console c++11  
     win32: LIBS \+= \-lopengl32

   * The CONFIG \+= console line ensures that a console window opens alongside the GUI to display debug messages from qDebug().  
3. **Build and Run**:  
   * From the Qt Creator menu, select Build \-\> Run qmake.  
   * Click the green "Run" button (or press Ctrl+R) to compile and start the application.

## **Algorithm Explained**

The mesh reconstruction is performed in three main steps:

### **Step 1: Build Adjacency Graph**

This is the most critical step. Instead of using a naive heuristic, the algorithm leverages precise, user-provided constraints.

* **Input**: A list of MeshPoint objects. Each object contains a QVector3D for its position and an int for its required\_neighbors.  
* **Process**: For each point P, the algorithm calculates the distance to all other points. It then sorts them by distance and selects exactly the P.required\_neighbors closest points to be its neighbors.  
* **Output**: A clean and accurate graph where each point is connected only to its true topological neighbors, as defined by the constraints.

### **Step 2: Find Faces**

This step identifies all valid structural faces from the adjacency graph.

* **Process**:  
  1. **4-Cycle Search**: It searches the graph for all closed loops of 4 points (e.g., P0-P1-P2-P3).  
  2. **Coplanarity Check**: It verifies that the four points of a cycle lie on the same plane within a small tolerance.  
  3. **Diagonal Length Heuristic**: To filter out non-structural "diagonal" faces, it checks if both diagonals of the quadrilateral (P0-P2 and P1-P3) are longer than any of its four edges. This is a strong indicator of a convex, structural face.  
* **Output**: A list of QuadFace objects that represent the "shell" of the hexahedral mesh.

### **Step 3: Build Hexahedra**

This final step assembles the valid faces into complete 3D cells.

* **Process**:  
  1. **Face Pairing**: The algorithm iterates through all possible pairs of faces from the list generated in Step 2\.  
  2. **Opposite Face Check**: For each pair, it performs rigorous checks to see if they can be the top and bottom faces of a hexahedron. This involves ensuring they share no vertices and are connected by exactly four "side" edges in the graph.  
  3. **Deduplication**: Since each hexahedron can be constructed from any of its 3 pairs of opposite faces, many candidates will be duplicates. A "signature" (a sorted list of the 8 vertex indices) is created for each candidate. Only hexahedra with a unique signature are added to the final list.  
* **Output**: A list of unique Hexahedron objects representing the fully reconstructed mesh.

## **How to Use the Application**

1. **Launch**: Run the application from Qt Creator.  
2. **Initial View**: You will see the initial point cloud in the 3D viewer.  
3. **Step 1**: Click the Step 1: Build Adjacency Graph button. The view will update to show lines connecting the points based on their neighbor constraints.  
4. **Step 2**: Click the Step 2: Find Faces button. The view will update to show all identified structural faces as semi-transparent blue quads.  
5. **Step 3**: Click the Step 3: Build Hexahedra button. The final, reconstructed hexahedra will be highlighted in semi-transparent red.  
6. **Reset**: Click Reset / Load Points at any time to return to the initial state.