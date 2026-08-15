# How few multiplications does a product need?

The rank of a bilinear map is the number of multiplications needed to compute
it. Strassen doing 2×2 matrix multiplication in seven instead of eight is where
fast matrix multiplication comes from, and finding such decompositions in
general is open.

This is a greedy search for them. It takes a bilinear map over `GF(p)` and looks
for a different spanning set whose slice ranks sum to less. The result is
allowed to have *more* slices than the map it rewrites — it only has to generate
the original, so more products of lower rank is a better answer. That is exactly
what Karatsuba's five products for a four-coefficient product is.

```sh
cmake -B build -G Ninja && cmake --build build
./build/rank/cpp/minimise-rank fixtures/f3_3x6.tensor
```

## Results

Polynomial multiplication of `n` coefficients by `m`. Times are cumulative and
sequential, on one core of an i5-12450H at 2.2 GHz — the machine Table 1 was
measured on. Numbers and timings: [`results.json`](results.json).

| Map | Naive | Step 1 | Step 2 | Step 3 | Internship |
|---|---|---|---|---|---|
| F2 5×5 | 25 | 16 | 14 | **14** · 2.52 s | 14 · 14.42 s |
| F2 3×8 | 24 | 19 | 16 | **15** · 5.95 s | 15 · 3460.54 s |
| F2 4×7 | 28 | 19 | 16 | **16** · 17.80 s | 16 · 5044.06 s |
| F3 3×6 | 18 | 12 | 11 | **10** · 9.92 s | 11, *did not finish* |

**One case improves on what was published.** F3 3×6 reaches 10 multiplications
where 11 was on record, because the internship's step 3 on that map never
terminated and the 11 is a step 2 figure from an abandoned run. That search now
finishes in under ten seconds.

**Step 3 is where almost all the cost is and almost none of the answer.** Across
the four maps it improved the result once, on 3×8. On 4×7 it spent 5020 of the
internship's 5044 seconds to return the rank step 2 already had. Work spent
making step 3 faster is work spent on the part that mostly does not pay — which
is the argument for looking at what it searches rather than how fast it
searches it.

## The three steps

1. **Greedy smallest basis.** Walk the span from lowest rank upwards, keeping
   whatever is not already spanned.
2. **Minimise over the map's own products.** Decompose each slice into rank-one
   maps and try recombining with those.
3. **Minimise over every rank-one map of the shape.** There are
   `(p^rows − 1)(p^cols − 1)/(p−1)²` of them: 961 for 5×5 over F2, 4732 for 3×6
   over F3. This is the expensive one.

Written out precisely, with the time and space cost of each and where the
scaling wall actually is: **[`method.md`](method.md)**. The short version is
that the limit is memory rather than time — step 1 materialises the whole span,
`Θ(p^k·n·m)`, and `k` grows as the search runs.

## What makes a result trustworthy

A search that quietly loses a slice reports excellent numbers, so the property
that matters is checked rather than assumed: after every step, in the tool and
not only in the tests, the result must still generate the map it came from. All
four results do.

Ties between equal-rank candidates break on enumeration order. Julia's default
sort is not stable, so the original left this to whatever quicksort did; fixing
it is what makes the published ranks reproducible rather than approximately
reachable.

## Where this stops

Every step is greedy and nothing here proves any result optimal. The bilinear
rank problem is open, and none of this settles it. The 10 for F3 3×6 is a better
decomposition than the one on record, not a claim about the true rank.
