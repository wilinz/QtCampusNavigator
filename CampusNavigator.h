#ifndef CAMPUSNAVIGATOR_H
#define CAMPUSNAVIGATOR_H

#include <QWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <vector>
#include <QString>

struct Building {
    int x;
    int y;
    QString name;
};

class DrawingArea : public QWidget {
    Q_OBJECT

public:
    DrawingArea(QWidget *parent = nullptr);

    void setBuildings(const std::vector<Building> &buildings);
    void setPaths(const std::vector<std::vector<int>> &walkAdjMatrix, const std::vector<std::vector<int>> &bikeAdjMatrix, const std::vector<std::vector<int>> &busAdjMatrix);
    void setCurrentPath(const std::vector<int> &path);

protected:
    void paintEvent(QPaintEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    std::vector<Building> buildings;
    std::vector<std::vector<int>> walkAdjMatrix;
    std::vector<std::vector<int>> bikeAdjMatrix;
    std::vector<std::vector<int>> busAdjMatrix;
    std::vector<int> currentPath;
    double scaleFactor = 1.0;

    void drawPaths(QPainter &painter);
    void drawLegend(QPainter &painter);
};

class CampusNavigator : public QWidget {
    Q_OBJECT

public:
    CampusNavigator(QWidget *parent = nullptr);

private slots:
    void calculatePath();

private:
    void createBuildings();
    void createEdges();

    QComboBox *travelModeComboBox;
    QListWidget *pathListWidget;
    QLineEdit *startLineEdit;
    QLineEdit *endLineEdit;
    QPushButton *navigateButton;
    DrawingArea *drawingArea;

    std::vector<Building> buildings;
    std::vector<std::vector<int>> walkAdjMatrix;
    std::vector<std::vector<int>> bikeAdjMatrix;
    std::vector<std::vector<int>> busAdjMatrix;
};

#endif // CAMPUSNAVIGATOR_H
