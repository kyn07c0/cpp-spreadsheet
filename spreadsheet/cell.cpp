#include "cell.h"

#include <string>
#include <optional>
#include <utility>

CellInterface::Value EmptyImpl::GetValue()
{
    return 0.0;
}

std::string EmptyImpl::GetText() const
{
    return "";
}

std::vector<Position> EmptyImpl::GetReferencedCells() const
{
    return {};
}


TextImpl::TextImpl(std::string text) : text_(std::move(text))
{
}

CellInterface::Value TextImpl::GetValue()
{
    if(text_[0] == ESCAPE_SIGN)
    {
        return text_.substr(1);
    }

    return text_;
}

std::string TextImpl::GetText() const
{
    return text_;
}

std::vector<Position> TextImpl::GetReferencedCells() const
{
    return {};
}


FormulaImpl::FormulaImpl(std::string text, SheetInterface& sheet) : sheet_(sheet)
{
    try
    {
        formula_ = ParseFormula(std::move(text));
    }
    catch (const std::exception& ex)
    {
        std::throw_with_nested(FormulaException(ex.what()));
    }
}


CellInterface::Value FormulaImpl::GetValue()
{
    FormulaInterface::Value value = formula_->Evaluate(sheet_);
    if(std::holds_alternative<double>(value))
    {
        cache_ = std::get<double>(value);
    }
    else
    {
        cache_ = std::get<FormulaError>(value);
    }

    return cache_;
}

std::string FormulaImpl::GetText() const
{
    return FORMULA_SIGN + formula_->GetExpression();
}

std::vector<Position> FormulaImpl::GetReferencedCells() const
{
    return formula_->GetReferencedCells();
}


Cell::Cell(SheetInterface& sheet) : impl_(std::make_unique<EmptyImpl>()), sheet_(sheet)
{
}

void Cell::Set(const std::string& text)
{
    if(text.empty())
    {
        impl_ = std::make_unique<EmptyImpl>();
    }
    else if(text[0] == FORMULA_SIGN && text.size() > 1)
    {
        impl_ = std::make_unique<FormulaImpl>(text.substr(1), sheet_);
    }
    else
    {
        impl_ = std::make_unique<TextImpl>(text);
    }
}

void Cell::Clear()
{
    impl_ = std::make_unique<EmptyImpl>();
    CacheInvalidation();
}

Cell::Value Cell::GetValue() const
{
    return impl_->GetValue();
}

std::string Cell::GetText() const
{
    std::string s = impl_->GetText();
    return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const
{
    return impl_->GetReferencedCells();
}

void Cell::AddDependentCell(Position pos)
{
    dependent_cells_.insert(pos);
}

void Cell::DeleteDependentCell(Position pos)
{
    dependent_cells_.erase(pos);
}


void Cell::CacheInvalidation()
{
    for(const auto& pos_dep : dependent_cells_)
    {
        auto cell_dep = dynamic_cast<Cell*>(sheet_.GetCell(pos_dep));
        cell_dep->CacheUpdate();
    }
}

void Cell::CacheUpdate()
{
    impl_->GetValue();
    CacheInvalidation();
}
