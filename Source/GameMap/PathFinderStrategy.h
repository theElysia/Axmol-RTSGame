#pragma once

#include <vector>
#include <unordered_set>
#include <optional>

class PathFinderStrategy
{
public:
    struct Point
    {
        int x, y;
        explicit Point(int xx = 0, int yy = 0) : x(xx), y(yy) {}
        Point operator+(const Point& rhs) const { return Point(x + rhs.x, y + rhs.y); }
        Point operator-(const Point& rhs) const { return Point(x - rhs.x, y - rhs.y); }
        bool operator==(const Point& rhs) const { return x == rhs.x && y == rhs.y; }
    };

public:
    virtual ~PathFinderStrategy() = default;

    virtual bool init(const std::vector<bool>& grid, int x, int y) = 0;

    virtual void setDest(const Point& dst) = 0;

    virtual std::optional<std::vector<Point>> findPath(const Point& src) = 0;
};

class PathFinder_AStar : public PathFinderStrategy
{
public:
    bool init(const std::vector<bool>& grid, int x, int y) override;
    void setDest(const Point& dst) override;
    std::optional<std::vector<Point>> findPath(const Point& src) override;

private:
    // bool isValid(const Point& p);
    int wx_ = 0, wy_ = 0;
    std::vector<bool> grid_;
};

class PathFinder_FlowField : public PathFinderStrategy
{
public:
    bool init(const std::vector<bool>& grid, int x, int y) override;
    void setDest(const Point& dst) override;
    std::optional<std::vector<Point>> findPath(const Point& src) override;

private:
    void resetField();
    int transPoint(const Point& p) { return p.x + p.y * wx_; }
    Point backToPoint(int p) { return Point(p % wx_, p / wx_); }
    void flowAlongTrace(const Point& src, std::vector<Point>& path);
    bool withinBoundary(const Point& p) { return p.x >= 0 && p.x < wx_ && p.y >= 0 && p.y < wy_; }

private:
    enum DirectionType
    {
        W,
        A,
        S,
        D,
        UNDEF,
    };

    static constexpr int direct_num = DirectionType::UNDEF;

    int wx_ = 0, wy_ = 0, wtotal_ = 0;
    Point dst_ = Point(-1, -1);
    std::vector<bool> grid_;
    std::vector<Point> neighbor_;
    std::vector<int> grid_index_;
    std::vector<DirectionType> grid_flow_;
};