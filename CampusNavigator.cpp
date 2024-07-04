#include "CampusNavigator.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <limits.h>
#include <algorithm>
#include <QScrollArea>
#include <QWheelEvent>
#include <QComboBox>
#include <set>

const int INF = INT_MAX;

DrawingArea::DrawingArea(QWidget *parent) : QWidget(parent) {
    setMinimumSize(1500, 1500);
}

void DrawingArea::setBuildings(const std::vector<Building> &buildings) {
    this->buildings = buildings;
    update();
}

void DrawingArea::setPaths(const std::vector<std::vector<int>> &walkAdjMatrix, const std::vector<std::vector<int>> &bikeAdjMatrix, const std::vector<std::vector<int>> &busAdjMatrix) {
    this->walkAdjMatrix = walkAdjMatrix;
    this->bikeAdjMatrix = bikeAdjMatrix;
    this->busAdjMatrix = busAdjMatrix;
    update();
}

void DrawingArea::setCurrentPath(const std::vector<int> &path) {
    this->currentPath = path;
    update();
}

void DrawingArea::paintEvent(QPaintEvent *event) {
    QWidget::paintEvent(event);
    QPainter painter(this);
    painter.scale(scaleFactor, scaleFactor);

    if (buildings.empty()) return;

    QFont font = painter.font();
    font.setPointSize(16);
    painter.setFont(font);

    for (size_t i = 0; i < buildings.size(); ++i) {
        painter.drawEllipse(buildings[i].x, buildings[i].y, 15, 15);
        painter.drawText(buildings[i].x + 10, buildings[i].y - 5, QString::number(i + 1));
        painter.drawText(buildings[i].x + 10, buildings[i].y + 20, buildings[i].name);
    }

    drawPaths(painter);
    drawLegend(painter);

    if (!currentPath.empty()) {
        painter.setPen(QPen(Qt::red, 2));
        for (size_t i = 0; i < currentPath.size() - 1; ++i) {
            int u = currentPath[i];
            int v = currentPath[i + 1];
            if (u >= 0 && u < buildings.size() && v >= 0 && v < buildings.size()) {
                painter.drawLine(buildings[u].x + 10, buildings[u].y + 10, buildings[v].x + 10, buildings[v].y + 10);
            }
        }
    }
}

void DrawingArea::drawPaths(QPainter &painter) {
    std::set<std::pair<int, int>> drawnEdges;

    auto drawEdge = [&](int i, int j, const QPen &pen, int distance) {
        std::pair<int, int> edge = std::minmax(i, j);
        if (drawnEdges.find(edge) == drawnEdges.end()) {
            drawnEdges.insert(edge);
            painter.setPen(pen);
            painter.drawLine(buildings[i].x + 10, buildings[i].y + 10, buildings[j].x + 10, buildings[j].y + 10);
            QPoint center = QPoint((buildings[i].x + buildings[j].x) / 2, (buildings[i].y + buildings[j].y) / 2);
            painter.drawText(center.x(), center.y() - 5, QString::number(distance) + "米");
        }
    };

    QPen walkPen(Qt::blue);
    QPen bikePen(Qt::green, 2, Qt::DashLine);
    QPen busPen(Qt::black, 3, Qt::DotLine);

    for (size_t i = 0; i < walkAdjMatrix.size(); ++i) {
        for (size_t j = i + 1; j < walkAdjMatrix[i].size(); ++j) {
            if (busAdjMatrix[i][j] != INF) {
                drawEdge(i, j, busPen, busAdjMatrix[i][j]);
            } else if (bikeAdjMatrix[i][j] != INF) {
                drawEdge(i, j, bikePen, bikeAdjMatrix[i][j]);
            } else if (walkAdjMatrix[i][j] != INF) {
                drawEdge(i, j, walkPen, walkAdjMatrix[i][j]);
            }
        }
    }
}

void DrawingArea::drawLegend(QPainter &painter) {
    int legendX = 20;
    int legendY = 20;

    QFont font = painter.font();
    font.setPointSize(16);
    painter.setFont(font);

    painter.setPen(QPen(Qt::blue));
    painter.drawLine(legendX, legendY, legendX + 20, legendY);
    painter.drawText(legendX + 25, legendY + 5, "步行道");

    legendY += 20;
    painter.setPen(QPen(Qt::green, 2, Qt::DashLine));
    painter.drawLine(legendX, legendY, legendX + 20, legendY);
    painter.drawText(legendX + 25, legendY + 5, "自行车道");

    legendY += 20;
    painter.setPen(QPen(Qt::black, 3, Qt::DotLine));
    painter.drawLine(legendX, legendY, legendX + 20, legendY);
    painter.drawText(legendX + 25, legendY + 5, "校车道");
}

void DrawingArea::wheelEvent(QWheelEvent *event) {
    if (event->angleDelta().y() > 0)
        scaleFactor *= 1.1;
    else
        scaleFactor /= 1.1;
    update();
}

CampusNavigator::CampusNavigator(QWidget *parent)
    : QWidget(parent), pathListWidget(new QListWidget(this)),
    startLineEdit(new QLineEdit(this)), endLineEdit(new QLineEdit(this)), navigateButton(new QPushButton("开始导航", this)),
    drawingArea(new DrawingArea(this)), travelModeComboBox(new QComboBox(this)) {

    createBuildings();
    createEdges();

    resize(1600, 1150);

    QScrollArea *scrollArea = new QScrollArea;
    scrollArea->setWidget(drawingArea);
    scrollArea->setWidgetResizable(true);

    travelModeComboBox->addItem("步行");
    travelModeComboBox->addItem("自行车");
    travelModeComboBox->addItem("校车");

    QVBoxLayout *mainLayout = new QVBoxLayout;
    QHBoxLayout *inputLayout = new QHBoxLayout;

    inputLayout->addWidget(new QLabel("输入起点:"));
    inputLayout->addWidget(startLineEdit);
    inputLayout->addWidget(new QLabel("输入终点:"));
    inputLayout->addWidget(endLineEdit);
    inputLayout->addWidget(new QLabel("出行方式:"));
    inputLayout->addWidget(travelModeComboBox);
    inputLayout->addWidget(navigateButton);

    mainLayout->addLayout(inputLayout);
    mainLayout->addWidget(scrollArea, 1);
    mainLayout->addWidget(pathListWidget);
    setLayout(mainLayout);

    connect(navigateButton, &QPushButton::clicked, this, &CampusNavigator::calculatePath);

    drawingArea->setBuildings(buildings);
    drawingArea->setPaths(walkAdjMatrix, bikeAdjMatrix, busAdjMatrix);
}

void CampusNavigator::createBuildings() {
    buildings = {
        {120, 200, "主教学楼"},
        {350, 150, "图书馆"},
        {620, 220, "实验楼"},
        {130, 450, "行政楼"},
        {480, 400, "学生中心"},
        {770, 350, "宿舍1"},
        {1050, 500, "宿舍2"},
        {230, 700, "体育馆"},
        {520, 750, "食堂"},
        {800, 600, "艺术楼"},
        {1120, 700, "医务室"},
        {1400, 500, "体育场"},
        {500, 1050, "计算机科学楼"},
        {820, 950, "化学楼"},
        {1200, 850, "工程楼"}
    };
}

void CampusNavigator::createEdges() {
    walkAdjMatrix = {
                     {0, 30, INF, 50, 80, INF, INF, INF, INF, INF, INF, INF, INF, INF, INF},
                     {30, 0, 80, INF, 70, INF, INF, INF, INF, INF, INF, INF, INF, INF, INF},
                     {INF, 80, 0, INF, 90, 60, INF, INF, INF, INF, INF, INF, INF, INF, INF},
                     {50, INF, INF, 0, 30, INF, INF, 60, INF, INF, INF, INF, INF, INF, INF},
                     {80, 70, 90, 30, 0, 50, INF, 90, 80, 80, INF, INF, INF, INF, INF},
                     {INF, INF, 60, INF, 50, 0, 80, INF, INF, 50, INF, INF, INF, INF, INF},
                     {INF, INF, INF, INF, INF, 80, 0, INF, INF, 50, 70, 50, INF, INF, INF},
                     {INF, INF, INF, 60, 90, INF, INF, 0, 80, 100, INF, INF, 120, INF, INF},
                     {INF, INF, INF, INF, 80, INF, INF, 80, 0, 50, INF, INF, 90, 90, INF},
                     {INF, INF, INF, INF, 80, 50, 50, 100, 50, 0, 70, INF, INF, 80, INF},
                     {INF, INF, INF, INF, INF, INF, 70, INF, INF, 70, 0, 50, INF, 100, 80},
                     {INF, INF, INF, INF, INF, INF, 50, INF, INF, INF, 50, 0, INF, INF, 60},
                     {INF, INF, INF, INF, INF, INF, INF, 120, 90, INF, INF, INF, 0, 30, INF},
                     {INF, INF, INF, INF, INF, INF, INF, INF, 90, 80, 100, INF, 30, 0, 80},
                     {INF, INF, INF, INF, INF, INF, INF, INF, INF, INF, 80, 60, INF, 80, 0}
    };

    bikeAdjMatrix = {
                     {0, 30, INF, 50, 80, INF, INF, INF, INF, INF, INF, INF, INF, INF, INF},
                     {30, 0, 80, INF, 70, INF, INF, INF, INF, INF, INF, INF, INF, INF, INF},
                     {INF, 80, 0, INF, 90, 60, INF, INF, INF, INF, INF, INF, INF, INF, INF},
                     {50, INF, INF, 0, 30, INF, INF, 60, INF, INF, INF, INF, INF, INF, INF},
                     {80, 70, 90, 30, 0, 50, INF, INF, 80, 80, INF, INF, INF, INF, INF},
                     {INF, INF, 60, INF, 50, 0, 80, INF, INF, 50, INF, INF, INF, INF, INF},
                     {INF, INF, INF, INF, INF, 80, 0, INF, INF, 50, 70, INF, INF, INF, INF},
                     {INF, INF, INF, 60, INF, INF, INF, 0, 80, 100, INF, INF, 120, INF, INF},
                     {INF, INF, INF, INF, 80, INF, INF, 80, 0, 50, INF, INF, 90, 90, INF},
                     {INF, INF, INF, INF, 80, 50, 50, 100, 50, 0, 70, INF, INF, INF, INF},
                     {INF, INF, INF, INF, INF, INF, 70, INF, INF, 70, 0, 50, INF, 100, 80},
                     {INF, INF, INF, INF, INF, INF, INF, INF, INF, INF, 50, 0, INF, INF, 60},
                     {INF, INF, INF, INF, INF, INF, INF, 120, 90, INF, INF, INF, 0, 30, INF},
                     {INF, INF, INF, INF, INF, INF, INF, INF, 90, 80, 100, INF, 30, 0, 80},
                     {INF, INF, INF, INF, INF, INF, INF, INF, INF, INF, INF, 60, INF, 80, 0}
    };

    busAdjMatrix = {
        {0, 30, INF, 50, INF, INF, INF, INF, INF, INF, INF, INF, INF, INF, INF},
        {30, 0, 80, INF, 70, INF, INF, INF, INF, INF, INF, INF, INF, INF, INF},
        {INF, 80, 0, INF, 90, 60, INF, INF, INF, INF, INF, INF, INF, INF, INF},
        {50, INF, INF, 0, 30, INF, INF, 60, INF, INF, INF, INF, INF, INF, INF},
        {INF, 70, 90, 30, 0, 50, INF, INF, 80, INF, INF, INF, INF, INF, INF},
        {INF, INF, 60, INF, 50, 0, 80, INF, INF, 50, INF, INF, INF, INF, INF},
        {INF, INF, INF, INF, INF, 80, 0, INF, INF, 50, 70, INF, INF, INF, INF},
        {INF, INF, INF, 60, INF, INF, INF, 0, 80, 100, INF, INF, INF, INF, INF},
        {INF, INF, INF, INF, 80, INF, INF, 80, 0, 50, INF, INF, 90, 90, INF},
        {INF, INF, INF, INF, INF, 50, 50, 100, 50, 0, 70, INF, INF, INF, INF},
        {INF, INF, INF, INF, INF, INF, 70, INF, INF, 70, 0, 50, INF, 100, 80},
        {INF, INF, INF, INF, INF, INF, INF, INF, INF, INF, 50, 0, INF, INF, 60},
        {INF, INF, INF, INF, INF, INF, INF, INF, 90, INF, INF, INF, 0, 30, INF},
        {INF, INF, INF, INF, INF, INF, INF, INF, 90, 80, 100, INF, 30, 0, 80},
        {INF, INF, INF, INF, INF, INF, INF, INF, INF, INF, INF, 60, INF, 80, 0}
    };
}

void CampusNavigator::calculatePath() {
    bool ok1, ok2;
    int start = startLineEdit->text().toInt(&ok1) - 1;
    int end = endLineEdit->text().toInt(&ok2) - 1;

    if (!ok1 || !ok2 || start < 0 || start >= 15 || end < 0 || end >= 15) {
        QListWidgetItem *item = new QListWidgetItem("请输入有效的起点和终点 (1-15)");
        pathListWidget->addItem(item);
        return;
    }

    std::vector<std::vector<int>> adjMatrix;
    QString travelMode = travelModeComboBox->currentText();
    if (travelMode == "步行") {
        adjMatrix = walkAdjMatrix;
    } else if (travelMode == "自行车") {
        adjMatrix = bikeAdjMatrix;
    } else if (travelMode == "校车") {
        adjMatrix = busAdjMatrix;
    }

    std::vector<int> dist(15, INF);
    std::vector<bool> visited(15, false);
    std::vector<int> prev(15, -1);

    dist[start] = 0;

    for (int i = 0; i < 15; ++i) {
        int u = -1;
        for (int j = 0; j < 15; j++) {
            if (!visited[j] && (u == -1 || dist[j] < dist[u])) {
                u = j;
            }
        }

        if (dist[u] == INF) break;
        visited[u] = true;

        for (int v = 0; v < 15; ++v) {
            if (adjMatrix[u][v] != INF && dist[u] + adjMatrix[u][v] < dist[v]) {
                dist[v] = dist[u] + adjMatrix[u][v];
                prev[v] = u;
            }
        }
    }

    if (dist[end] == INF) {
        QListWidgetItem *item = new QListWidgetItem("无法从起点到达终点");
        pathListWidget->addItem(item);
        return;
    }

    QString pathDescription = QString("最短路径: ");
    std::vector<int> path;
    for (int at = end; at != -1; at = prev[at])
        path.push_back(at);
    std::reverse(path.begin(), path.end());
    for (size_t i = 0; i < path.size(); ++i) {
        if (i != 0) pathDescription += " -> ";
        pathDescription += QString::number(path[i] + 1);
    }
    pathDescription += QString(", 路程: %1 米").arg(dist[end]);

    QListWidgetItem *item = new QListWidgetItem(pathDescription);
    pathListWidget->addItem(item);

    drawingArea->setCurrentPath(path);
    drawingArea->update();
}
