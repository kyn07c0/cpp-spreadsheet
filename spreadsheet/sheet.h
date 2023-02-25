#pragma once

#include "cell.h"
#include "common.h"

#include <functional>
#include <unordered_map>

class Cell;

struct Hasher
{
    size_t operator()(Position pos) const;
};

class Sheet : public SheetInterface
{
public:
    ~Sheet() override;

    void SetCell(Position pos, const std::string& text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

    void ResizeTable();
    bool CheckCircularDependency(Position pos, const std::vector<Position>& dep_cells);

private:
    void CheckPosition(Position pos) const;

    Size table_size_= {0, 0};
    std::unordered_map<Position, std::unique_ptr<Cell>, Hasher> table_;
};
