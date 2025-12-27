#pragma once

// позиция в сетке и общий интерфейс обновления



class GameObject {
protected:
    // позиция объекта в клетках
    int row;
    int col;

public:
    // создание объекта в заданной клетке
    GameObject(int row, int col)
        : row(row), col(col)
    {
    }

    virtual ~GameObject() = default;

    // текущая позиция
    int getRow() const { return row; }
    int getCol() const { return col; }

    // изменение позиции
    void setRow(int r) { row = r; }
    void setCol(int c) { col = c; }

    virtual void update(float dt) = 0;
};
