#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <vector>
#include "reconstruction_engine.h"

// Forward declaration
class GLWidget;
class QPushButton;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onReset();
    void onStep1_BuildGraph();
    void onStep2_FindFaces();
    void onStep3_BuildHexahedra();

private:
    void setupUI();

    // UI Widgets
    GLWidget *m_glWidget;
    QPushButton *m_resetButton;
    QPushButton *m_step1Button;
    QPushButton *m_step2Button;
    QPushButton *m_step3Button;

    // Data containers for the reconstruction process
    std::vector<MeshPoint> m_points;
    AdjacencyGraph m_adjGraph;
    std::vector<QuadFace> m_faces;
    std::vector<Hexahedron> m_hexahedra;
};
#endif // MAINWINDOW_H
