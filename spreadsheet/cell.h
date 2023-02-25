#pragma once

#include "common.h"
#include "formula.h"
#include "sheet.h"

#include <functional>
#include <unordered_set>
#include <set>

class Sheet;

class Impl
{
public:
    virtual CellInterface::Value GetValue() = 0;
    virtual std::string GetText() const = 0;
    virtual std::vector<Position> GetReferencedCells() const = 0;
};

class EmptyImpl : public Impl
{
public:
    EmptyImpl();

    CellInterface::Value GetValue() override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;
};

class TextImpl : public Impl
{
public:
    explicit TextImpl(std::string text);

    CellInterface::Value GetValue() override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;

private:
    std::string text_;
};

class FormulaImpl : public Impl
{
public:
    FormulaImpl(std::string text, SheetInterface& sheet);

    CellInterface::Value GetValue() override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;

private:
    std::unique_ptr<FormulaInterface> formula_;
    SheetInterface& sheet_;
    CellInterface::Value cache_;
};


class Cell : public CellInterface
{
public:
    explicit Cell(SheetInterface& sheet);
    ~Cell() override;

    void Set(const std::string& text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;

    std::vector<Position> GetReferencedCells() const override;

    void AddDependentCell(Position pos);
    void DeleteDependentCell(Position pos);

    void CacheInvalidation();
    void CacheUpdate();

private:
    std::unique_ptr<Impl> impl_ = nullptr;
    SheetInterface& sheet_;
    std::set<Position> dependent_cells_;
};
