#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cctype>
#include <sstream>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe)
{
    return output << fe.ToString();
}

namespace {

    class Formula : public FormulaInterface
    {
    public:
        explicit Formula(const std::string& expression) try : ast_(ParseFormulaAST(expression))
        {
        }
        catch (const std::exception& ex)
        {
            std::throw_with_nested(FormulaException(ex.what()));
        }

        Value Evaluate(const SheetInterface& sheet) const override
        {
            Value value = 0.0;

            try
            {
                value = ast_.Execute([&sheet](Position pos){

                    auto cell = sheet.GetCell(pos);
                    if(cell == nullptr)
                    {
                        return 0.0;
                    }

                    auto value = cell->GetValue();
                    if(std::holds_alternative<double>(value))
                    {
                        return std::get<double>(value);
                    }
                    else if(std::holds_alternative<std::string>(value))
                    {
                        std::string str = std::get<std::string>(value);

                        if(str.empty())
                        {
                            return 0.0;
                        }

                        if(!std::all_of(str.begin(), str.end(), ::isdigit))
                        {
                            throw FormulaError(FormulaError::Category::Value);
                        }

                        return std::stod(str);
                    }
                    else if(std::holds_alternative<FormulaError>(value))
                    {
                        throw std::get<FormulaError>(value);
                    }

                    return 0.0;
                });
            }
            catch(const FormulaError& fe)
            {
                return fe;
            }

            return value;
        }

        std::string GetExpression() const override
        {
            std::ostringstream ss;
            ast_.PrintFormula(ss);

            return ss.str();
        }

        std::vector<Position> GetReferencedCells() const override
        {
            auto cells = ast_.GetCells();
            std::vector<Position> cells_ref(cells.begin(), cells.end());
            cells_ref.erase(unique(cells_ref.begin(), cells_ref.end()), cells_ref.end());

            return cells_ref;
        }

    private:
        FormulaAST ast_;
    };

}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(const std::string& expression)
{
    return std::make_unique<Formula>(std::move(expression));
}
