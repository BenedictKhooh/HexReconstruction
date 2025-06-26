#include "mainwindow.h"
#include "glwidget.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setupUI();

    // Initialize the point data with the new 3x1x1 structure and corrected neighbor constraints.
    m_points = {
        // pos, required_neighbors
        // Bottom layer (z=0), corners of the whole block, 3 neighbors each
        { {-0.2, 0, 0}, 3 }, { {1, 0, 0}, 3 }, { {1, 1, 0}, 3 }, { {0, 1, 0}, 3 },
        // First internal layer (z=1), shared by two cubes, 4 neighbors each
        { {0, 0, 1}, 4 }, { {1, 0, 1}, 4 }, { {1, 1, 1}, 4 }, { {0, 1, 1}, 4 },
        // Second internal layer (z=2), shared by two cubes, 4 neighbors each
        { {0, 0, 2}, 4 }, { {1, 0, 2}, 4 }, { {1, 1, 2}, 4 }, { {0, 1, 2}, 4 },
        // Top layer (z=3), corners of the whole block, 3 neighbors each
        { {0, 0, 3}, 3 }, { {1, 0, 3}, 3 }, { {1, 1, 3}, 3 }, { {0, 1, 3}, 3 }
    };

    onReset(); // Set initial state
}

MainWindow::~MainWindow() {}

// Set up the main window layout and connect signals to slots.
void MainWindow::setupUI() {
    setWindowTitle("Hexahedral Mesh Reconstruction (3D Viewer)");
    m_glWidget = new GLWidget(this);
    m_resetButton = new QPushButton("Reset / Load Points", this);
    m_step1Button = new QPushButton("Step 1: Build Adjacency Graph", this);
    m_step2Button = new QPushButton("Step 2: Find Faces", this);
    m_step3Button = new QPushButton("Step 3: Build Hexahedra", this);

    // Connect button clicks to their respective handler functions (slots).
    connect(m_resetButton, &QPushButton::clicked, this, &MainWindow::onReset);
    connect(m_step1Button, &QPushButton::clicked, this, &MainWindow::onStep1_BuildGraph);
    connect(m_step2Button, &QPushButton::clicked, this, &MainWindow::onStep2_FindFaces);
    connect(m_step3Button, &QPushButton::clicked, this, &MainWindow::onStep3_BuildHexahedra);

    // Set up layouts.
    QVBoxLayout *controlLayout = new QVBoxLayout;
    controlLayout->addWidget(m_resetButton);
    controlLayout->addWidget(m_step1Button);
    controlLayout->addWidget(m_step2Button);
    controlLayout->addWidget(m_step3Button);
    controlLayout->addStretch();
    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(m_glWidget, 1); // GL widget takes most of the space
    mainLayout->addLayout(controlLayout);
    QWidget *centralWidget = new QWidget(this);
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);
    resize(1024, 768);
}

// Slot for the Reset button.
void MainWindow::onReset() {
    // Clear all intermediate and final data.
    m_adjGraph.clear();
    m_faces.clear();
    m_hexahedra.clear();

    // Reset the GL widget and load the initial points.
    m_glWidget->reset();
    m_glWidget->setPoints(m_points);

    // Reset button states for the step-by-step process.
    m_step1Button->setEnabled(true);
    m_step2Button->setEnabled(false);
    m_step3Button->setEnabled(false);
    qDebug() << "--- System reset. Points loaded. ---";
}

// Slot for the Step 1 button.
void MainWindow::onStep1_BuildGraph() {
    qDebug() << "--- Executing Step 1: Building Adjacency Graph ---";
    m_adjGraph = ReconstructionEngine::buildAdjacencyGraph(m_points);
    m_glWidget->setAdjacencyGraph(m_adjGraph);

    m_step1Button->setEnabled(false);
    m_step2Button->setEnabled(true);
    qDebug() << "Adjacency graph built.";
}

// Slot for the Step 2 button.
void MainWindow::onStep2_FindFaces() {
    qDebug() << "--- Executing Step 2: Finding Faces ---";
    m_faces = ReconstructionEngine::findValidFaces(m_points, m_adjGraph);
    m_glWidget->setFaces(m_faces);

    m_step2Button->setEnabled(false);
    m_step3Button->setEnabled(true);
    qDebug() << "Found" << m_faces.size() << "valid faces.";
}

// Slot for the Step 3 button.
void MainWindow::onStep3_BuildHexahedra() {
    qDebug() << "--- Executing Step 3: Building Hexahedra ---";
    m_hexahedra = ReconstructionEngine::buildHexahedra(m_faces, m_adjGraph);
    m_glWidget->setHexahedra(m_hexahedra);

    m_step3Button->setEnabled(false);
    qDebug() << "Reconstructed" << m_hexahedra.size() << "hexahedra.";
}
