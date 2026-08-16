#include "integer_programme.h"

#include "whole_numbers.h"

namespace optimisation {

namespace {

bool holds(Relation relation, const Number& left, const Number& right) {
    switch (relation) {
        case Relation::LessOrEqual: return left <= right;
        case Relation::GreaterOrEqual: return left >= right;
        case Relation::Equal: return left == right;
    }
    return false;
}

Number coefficient_at(const std::vector<Number>& coefficients, std::size_t index) {
    return index < coefficients.size() ? coefficients[index] : Number(0);
}

}  // namespace

bool satisfies(const IntegerProgramme& programme, const std::vector<Number>& values) {
    if (values.size() != programme.variables.size()) return false;

    for (std::size_t index = 0; index < values.size(); ++index) {
        const Variable& variable = programme.variables[index];
        if (variable.bounded_below && values[index] < variable.lower) return false;
        if (variable.bounded_above && values[index] > variable.upper) return false;
        if (variable.integral && !is_whole(values[index])) return false;
    }

    for (const Constraint& constraint : programme.constraints) {
        Number total = Number(0);
        for (std::size_t index = 0; index < values.size(); ++index) {
            total += coefficient_at(constraint.coefficients, index) * values[index];
        }
        if (!holds(constraint.relation, total, constraint.bound)) return false;
    }
    return true;
}

Number objective_at(const IntegerProgramme& programme, const std::vector<Number>& values) {
    Number total = Number(0);
    for (std::size_t index = 0; index < values.size(); ++index) {
        total += coefficient_at(programme.objective, index) * values[index];
    }
    return total;
}

bool improves(Sense sense, const Number& candidate, const Number& incumbent) {
    return sense == Sense::Minimise ? candidate < incumbent : candidate > incumbent;
}

}  // namespace optimisation
