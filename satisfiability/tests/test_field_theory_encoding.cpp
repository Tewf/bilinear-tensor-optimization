/// The SMT-LIB backend: that it says what it means, and reads back what it is
/// told.
///
/// This one cannot be checked the way the two CNF encodings are. There is no
/// clause to evaluate, because the whole point is that the field is the
/// solver's and not ours — so what is checked here is the shape of the problem,
/// the concrete syntax of the theory, and the round trip through a model. The
/// real check is agreeing with the one-hot encoder on the same tensors, and
/// that needs cvc5 installed.
#include <sstream>
#include <string>

#include "check.h"
#include "field_theory_encoding.h"

namespace {

using satisfiability::Field;
using satisfiability::Matrix;

linear_algebra::Tensor small_ternary(const Field& field) {
    linear_algebra::Tensor tensor;
    tensor.characteristic = 3;
    tensor.slices.assign(2, Matrix(2, 2));
    field.init(tensor.slices[0](0, 0), 1);
    field.init(tensor.slices[1](1, 1), 2);
    return tensor;
}

std::string written(const satisfiability::FieldTheoryEncoding& encoding) {
    std::ostringstream out;
    linear_algebra::write_smtlib(out, encoding.problem);
    return out.str();
}

std::size_t occurrences(const std::string& haystack, const std::string& needle) {
    std::size_t found = 0;
    for (std::size_t at = haystack.find(needle); at != std::string::npos;
         at = haystack.find(needle, at + needle.size())) {
        ++found;
    }
    return found;
}

}  // namespace

int main() {
    const Field field(3);
    const auto tensor = small_ternary(field);
    const auto encoding = satisfiability::encode_field_rank_at_most(tensor, 2);

    check::equal("a constant per unknown",
                 static_cast<long long>(encoding.problem.constants.size()),
                 static_cast<long long>(2 * (2 + 2 + 2)));
    check::equal("an assertion per tensor entry",
                 static_cast<long long>(encoding.problem.assertions.size()), 8);

    const std::string text = written(encoding);
    check::equal("the logic is quantifier-free finite fields",
                 text.find("(set-logic QF_FF)") != std::string::npos ? 1 : 0, 1);
    check::equal("the sort names the characteristic",
                 text.find("(define-sort F () (_ FiniteField 3))") != std::string::npos ? 1 : 0, 1);
    check::equal("constants are declared over it",
                 text.find("(declare-const a_0_0 F)") != std::string::npos ? 1 : 0, 1);
    check::equal("models are asked for",
                 text.find("(set-option :produce-models true)") != std::string::npos ? 1 : 0, 1);
    check::equal("it ends by asking", text.find("(check-sat)") != std::string::npos ? 1 : 0, 1);

    // Two products, so every assertion adds two triple products.
    check::equal("two summands per assertion", static_cast<long long>(occurrences(text, "ff.add")),
                 8);
    check::equal("two multiplications per summand",
                 static_cast<long long>(occurrences(text, "ff.mul")), 32);

    // The entry that is 2 must be asserted equal to the literal 2.
    check::equal("a nonzero entry is asserted against its own literal",
                 text.find("(as ff2 F)") != std::string::npos ? 1 : 0, 1);
    check::equal("field literals are written as the theory writes them",
                 linear_algebra::SmtProblem::literal(2) == "(as ff2 F)" ? 1 : 0, 1);

    // Reading a model back, in the syntax cvc5 prints.
    std::istringstream answer(
        "sat\n"
        "(\n"
        "(define-fun a_0_0 () (_ FiniteField 3) #f1m3)\n"
        "(define-fun b_0_0 () (_ FiniteField 3) #f1m3)\n"
        "(define-fun c_0_0 () (_ FiniteField 3) #f1m3)\n"
        ")\n");
    const auto model = linear_algebra::read_smtlib_model(answer);
    check::equal("sat is read", model.satisfiable ? 1 : 0, 1);
    check::equal("a value is read", static_cast<long long>(model.values.at("a_0_0")), 1);

    std::istringstream refused("unsat\n");
    const auto no = linear_algebra::read_smtlib_model(refused);
    check::equal("unsat is answered", no.answered ? 1 : 0, 1);
    check::equal("unsat is not satisfiable", no.satisfiable ? 1 : 0, 0);

    std::istringstream silent("(error \"killed\")\n");
    const auto quiet = linear_algebra::read_smtlib_model(silent);
    check::equal("no verdict is not unsat", quiet.answered ? 1 : 0, 0);

    return check::report("field theory encoding");
}
