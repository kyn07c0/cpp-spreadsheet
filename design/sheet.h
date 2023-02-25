#pragma once

#include "cell.h"
#include "common.h"

#include <functional>

struct Hasher 
{
    size_t operator()(Position pos) const;
};

class Sheet : public SheetInterface
{
public:
    ~Sheet() override;

    void SetCell(Position pos, const std::string text) override; // const установил, а вот ссылку не удалось, т.к. переопределяется метод интерфейса SheetInterface, которы нельзя менять   

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;
    
    void ResizeTable(Position pos);
    bool CheckCircularDependency(Position pos, std::vector<Position> dep_cells);

private:
    void CheckPosition(Position pos) const;

    Size table_size_= {0, 0};
    std::unordered_map<Position, std::unique_ptr<Cell>, Hasher> table_;
};
