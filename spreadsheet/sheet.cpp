#include "sheet.h"

#include "cell.h"
#include "common.h"
#include <iostream>
#include <optional>

using namespace std::literals;

size_t Hasher::operator()(Position pos) const
{
    return std::hash<int>{}(pos.row) + std::hash<int>{}(pos.col);
}

Sheet::~Sheet() = default;

void Sheet::SetCell(Position pos, const std::string& text)
{
    CheckPosition(pos);

    auto cell = dynamic_cast<Cell*>(GetCell(pos));

    // Ничего не делаем если содержание ячейки не изменилось
    if(cell != nullptr && cell->GetText() == text)
    {
        return;
    }

    // Проверка на циключескую зависимость
    if(text[0] == FORMULA_SIGN && text.size() > 1)
    {
        auto dep_cells = ParseFormula(text.substr(1))->GetReferencedCells();
        if(CheckCircularDependency(pos, dep_cells))
        {
            throw CircularDependencyException("Formula Circular Dependency");
        }
    }

    // Добавление новой или изменение существующей ячейки в таблице
    if(!table_.count(pos))
    {
        table_[pos] = std::make_unique<Cell>(*this);
        cell = dynamic_cast<Cell*>(GetCell(pos));
    }
    else
    {
        // Удаление информации об этой ячейки в других ячейках
        for(const auto& pos_ref : cell->GetReferencedCells())
        {
            auto cell_ref = dynamic_cast<Cell*>(GetCell(pos_ref));
            if(cell_ref != nullptr)
            {
                cell_ref->DeleteDependentCell(pos);
            }
        }
    }
    table_[pos]->Set(text);
    ResizeTable();

    // Получение списка ячеек, на которые есть ссылки в формуле
    for(const Position pos_ref : cell->GetReferencedCells())
    {
        // Если ссылка указывает на несозданную ячейка, создаем пустую ячейку
        if(dynamic_cast<Cell*>(GetCell(pos_ref)) == nullptr)
        {
            SetCell(pos_ref, "");
        }

        auto cell_ref = dynamic_cast<Cell*>(GetCell(pos_ref));

        // Добавить инф. о текущей ячейки в зависимую ячейку
        cell_ref->AddDependentCell(pos);
    }

    // Инвалидация кэша
    cell->CacheInvalidation();
}

CellInterface* Sheet::GetCell(Position pos)
{
    CheckPosition(pos);

    if(table_.count(pos))
    {
        return table_.at(pos).get();
    }

    return nullptr;
}

const CellInterface* Sheet::GetCell(Position pos) const
{
    CheckPosition(pos);

    if(table_.count(pos))
    {
        return table_.at(pos).get();
    }

    return nullptr;
}

void Sheet::ClearCell(Position pos)
{
    CheckPosition(pos);

    if(table_.count(pos) == 0)
    {
        return;
    }

    table_.erase(pos);
    ResizeTable();
}

Size Sheet::GetPrintableSize() const
{
    return table_size_;
}

void Sheet::PrintValues(std::ostream& output) const
{
    for(int row = 0; row < table_size_.rows; ++row)
    {
        for(int col = 0; col < table_size_.cols; ++col)
        {
            const auto *cell = reinterpret_cast<const Cell*>(GetCell({row, col}));
            if(cell != nullptr)
            {
                Position pos{row, col};
                auto value = table_.at(pos)->GetValue();
                if(std::holds_alternative<double>(value))
                {
                    output << std::get<double>(value);
                }
                else if(std::holds_alternative<std::string>(value))
                {
                    output << std::get<std::string>(value);
                }
                else if(std::holds_alternative<FormulaError>(value))
                {
                    output << std::get<FormulaError>(value);
                }
            }

            if(col < table_size_.cols - 1)
            {
                output << "\t";
            }
        }
        output << "\n";
    }
}

void Sheet::PrintTexts(std::ostream& output) const
{
    for(int row = 0; row < table_size_.rows; ++row)
    {
        for(int col = 0; col < table_size_.cols; ++col)
        {
            const auto *cell = reinterpret_cast<const Cell*>(GetCell({row, col}));
            if(cell != nullptr)
            {
                output << cell->GetText();
            }

            if(col < table_size_.cols - 1)
            {
                output << "\t";
            }
        }
        output << "\n";
    }
}

std::unique_ptr<SheetInterface> CreateSheet()
{
    return std::make_unique<Sheet>();
}

void Sheet::ResizeTable()
{
    table_size_ = {0,0};
    for(const auto&[pos, cell] : table_)
    {
        table_size_.rows = std::max(table_size_.rows, pos.row + 1);
        table_size_.cols = std::max(table_size_.cols, pos.col + 1);
    }
}

bool Sheet::CheckCircularDependency(Position pos, const std::vector<Position> &dep_cells)
{
    if(dep_cells.empty())
    {
        return false;
    }

    if(std::find(dep_cells.begin(), dep_cells.end(), pos) != dep_cells.end())
    {
        return true;
    }

    for(const auto& dep_cell : dep_cells)
    {
        auto child_cell = GetCell(dep_cell);

        if(child_cell == nullptr)
        {
            continue;
        }

        auto child_dep_cells = child_cell->GetReferencedCells();

        if(CheckCircularDependency(pos, child_dep_cells))
        {
            return true;
        }
    }

    return false;
}

void Sheet::CheckPosition(Position pos) const
{
    if(!pos.IsValid())
    {
        throw InvalidPositionException("Position is invalid");
    }
}
