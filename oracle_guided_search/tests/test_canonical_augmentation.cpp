#include <string>

#include "candidate_pool.h"
#include "canonical_augmentation.h"
#include "check.h"
#include "group_construction.h"
#include "tensor_file.h"

/// The one target that settles whether the deduplication is right.
///
/// `⟨2,2,2⟩` at 7 has 36 solution subspaces in a single orbit under its 216-element
/// group, computed twice by independent means and agreeing. So the plain enumerator
/// must find 36 and the canonical one must find 1, and any other pair means the
/// parent test is wrong rather than merely slow. The single orbit is de Groote's
/// uniqueness theorem for `⟨2,2,2⟩` recovered from the tensor.
int main(int argc, char** argv) {
    const std::string directory = argc > 1 ? argv[1] : "fixtures";
    const linear_algebra::Tensor strassen =
        linear_algebra::read_tensor_file(directory + "/matmul_2x2x2.tensor");
    const bilinear_rank::Field field(strassen.characteristic);

    const std::vector<bilinear_rank::Matrix> pool =
        bilinear_rank::all_rank_one_maps(field, strassen.rows(), strassen.columns());
    const std::vector<bilinear_rank::Automorphism> group =
        bilinear_rank::matrix_multiplication_symmetries(field, 2, 2, 2);
    check::equal("pool of <2,2,2>", pool.size(), 225);
    check::equal("group of <2,2,2>", group.size(), 216);

    const bilinear_rank::EnumerationReport plain =
        bilinear_rank::enumerate_solution_subspaces(field, strassen, pool, group, 7, false);
    check::equal("plain enumeration finds every solution subspace", plain.distinct, 36);
    check::equal("reaching each of them once per basis of the quotient", plain.emitted, 720);

    const bilinear_rank::EnumerationReport canonical =
        bilinear_rank::enumerate_solution_subspaces(field, strassen, pool, group, 7, true);
    check::equal("canonical augmentation finds one per orbit", canonical.distinct, 1);
    check::equal("and reaches it exactly once", canonical.emitted, 1);

    // Every emitted basis was multiplied out against the map inside the enumerator,
    // so reaching here means all of them compute <2,2,2>. Its size is the other half.
    check::equal("and it is a seven-product algorithm", canonical.decompositions.front().size(), 7);
    return check::report("canonical_augmentation");
}
