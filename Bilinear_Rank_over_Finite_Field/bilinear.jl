using LinearAlgebra
using IterTools
using Base.Iterators: product

function matrix_rank(matrix, p)
    matrix = mod.(matrix, p)
    rank, _ = row_reduce(matrix, p)
    return rank
end

function mod_inv(a, p)
    return invmod(a,p)
end

function row_reduce(matrix, p)
    matrix = mod.(matrix, p)
    rows, cols = size(matrix)
    rank = 0

    for col in 1:cols
        if rank >= rows
            break
        end

        pivot_row = nothing
        for row in rank+1:rows
            if matrix[row, col] != 0
                pivot_row = row
                break
            end
        end

        if pivot_row === nothing
            continue
        end

        if pivot_row != rank + 1
            matrix[[rank+1, pivot_row], :] = matrix[[pivot_row, rank+1], :]
        end

        pivot_value = matrix[rank+1, col]
        pivot_inv = mod_inv(pivot_value, p)
        matrix[rank+1, :] = mod.(matrix[rank+1, :] * pivot_inv, p)

        for row in rank+2:rows
            factor = matrix[row, col]
            matrix[row, :] = mod.(matrix[row, :] - factor * matrix[rank+1, :], p)
        end

        rank += 1
    end

    for col in cols:-1:1
        for row in rank:-1:1
            if matrix[row, col] != 0
                for elim_row in 1:row-1
                    factor = matrix[elim_row, col]
                    matrix[elim_row, :] = mod.(matrix[elim_row, :] - factor * matrix[row, :], p)
                end
                break
            end
        end
    end

    return rank, matrix
end

function is_linearly_independent(matrices, new_matrix, p)
    if isempty(matrices)
        return any(new_matrix .!= 0)
    end

    flattened_matrices = [vec(m) for m in matrices]

    flattened_new_matrix = vec(new_matrix)

    combined_matrix = hcat(flattened_matrices...)

    extended_matrix = hcat(combined_matrix, flattened_new_matrix)

    original_rank = matrix_rank(combined_matrix, p)
    extended_rank = matrix_rank(extended_matrix, p)

    if all(new_matrix .== 0)
        return extended_rank > original_rank
    else
        return extended_rank == original_rank + 1
    end
end

function Span_it(W, T, p)
    for t in T
        if is_linearly_independent(W, t, p)
            return false
        end
    end
    return true
end

function Number_of_multiplications(W, p)
    s = 0
    for j in W
        s += matrix_rank(j, p)
    end
    return s
end

function generate_arrays(m, p)
    s=[]
    for i in collect(product(fill(0:p-1, m)...))
        s=vcat(s,[collect(i)])
    end
    return s
end

function my_product(list0, n,p)
    result = [[]]
    for _ in 1:n
        temp = []
        for combination in result
            for element in list0
                comb_array = vcat([combination...], [element])
                if matrix_rank(hcat(comb_array...),p) <= 1
                    push!(temp, vcat(combination, [element]))
                end
            end
        end
        result = temp
    end
    return result
end

function independ(x, p)
    s = []
    for i in x
        k = true
        for j in s
            if !(is_linearly_independent([j], hcat(i...), p))
                k = false
                break
            end
        end
        if k && any(hcat(i...) .!= 0)
            push!(s, hcat(i...))
        end
    end
    return s
end

function generate_G(n, m, p)
    return independ(my_product(generate_arrays(n, p), m,p),p)
end

function create_of_zeroes(n, m)
    s = [zeros(Int, n, m) for _ in 1:(n+m-1)]
    return s
end

function create_Tensor_model_of_polynomial_multiplication(n, m)
    T = create_of_zeroes(n, m)
    for i in 1:(n+m-1)
        for j in 1:n
            for k in 1:m
                if k+j-1 == i
                    T[i][j, k] = 1
                end
            end
        end
    end
    return T
end

function tensor_modular(x, y, p)
    inverse = mod_inv(y[1], p)
    i = 1
    
    while i <= length(x) - length(y)+1 && matrix_rank(x[i],p) == 0
        i += 1
    end
    
    while i <= length(x) - length(y)+1
        x = subtract(x, y, i, x[i] * inverse, p)
        while i <= length(x) - length(y)+1 && matrix_rank(x[i],p) == 0
            i += 1
        end
    end
    
    return filter(i -> matrix_rank(i,p) != 0, x)
end
function subtract(x, y, i, factor, p)
    y_length = length(y)
    for j in 1:y_length
        x[i + j - 1] = (x[i + j - 1] - y[j] * factor) 
    end
    return x
end

function gaussian_elimination_over_p(A, U,p)
    m, n = size(A)
    # Create the augmented matrix by concatenating A and U
    augmented_matrix = hcat(A, U)

    for j in 1:n
        # Find the pivot row
        pivot_row = nothing
        for k in j:m
            if augmented_matrix[k, j]!= 0
                pivot_row = k
                break
            end
        end

        if pivot_row === nothing
            # No pivot in this column, skip to the next column
            continue
        end

        # Swap current row with pivot row if needed
        if pivot_row!= j
            augmented_matrix[[j, pivot_row], :] = augmented_matrix[[pivot_row, j], :]
        end

        # Normalize the pivot row
        augmented_matrix[j, :].*= mod_inv(augmented_matrix[j, j],p)

        # Eliminate the current column in all other rows
        for i in 1:m
            if i!= j
                augmented_matrix[i, :] -= augmented_matrix[i, j] * augmented_matrix[j, :]
            end
        end
    end

    augmented_matrix=augmented_matrix.%p
    # Check for inconsistency
    for i in (n+1):m
        if augmented_matrix[i, end]!= 0
            return nothing  # No solution
        end
    end

    return augmented_matrix[1:n, end]
end

function matrix_rank1_decomposer(x, p)
    n = size(x, 1)
    k = 1

    # Find the first non-zero row
    while k <= n && all(x[k, :] .== 0)
        k += 1
    end

    if k > n
        return [] # The matrix is zero
    end

    # Initialize the decomposition
    s = [[x[k, :], zeros(Int, n)]]
    s[1][2][k] = 1

    for i in k+1:n
        coeffs = gaussian_elimination_over_p(vcat(transpose.([row[1] for row in s])...)', x[i, :], p)
        if coeffs == nothing
            push!(s, [x[i, :], zeros(Int, n)])
            s[end][2][i] = 1
        else
            for t in 1:length(s)
                    s[t][2][i] = mod(s[t][2][i] + coeffs[t], p)
                end
        end
    end

    return [kron(j[2],j[1]').%p for j in s]
end
function rank1_base(x, p)
    s=[]
    for i in x
        s=vcat(s,matrix_rank1_decomposer(i,p))
    end
    return s
end

# Function to multiply two vectors and return the result modulo p
function multiplier(x, y, p)
    s = x[1] * y[1]
    for i in 2:length(x)
        s += x[i] * y[i]
    end
    return s 
end

# Function to check and find the solution with reduced rank
 function span_of(T,p)
    G=generate_arrays(length(T),p)
    s=[]
    for i in G
        s = vcat(s, [multiplier(i, T, p)])
    end
    return sort(s, by = x -> matrix_rank(x,p))
end

function smallest_base(T,p)
    G=span_of(T,p)
    s=[]
    for i in G
        if is_linearly_independent(s, i, p)
            s=vcat(s, [i])
        end
    end
    return s
end

function rank_minimizer(T,G,p)
    for i in 1:length(G)
        if is_linearly_independent(T, G[i], p)
            v=smallest_base(vcat(T, [G[i]]), p)
            if Number_of_multiplications(v,p)<Number_of_multiplications(T,p)
                T=v
            else
    
                G_new=[j for j in G[i+1:end] if is_linearly_independent(v, j, p)]
                return rank_minimizer(T,G_new,p)
            end
        end
    end
    return T
end

function filter(T,G,p, s=[])
    for i in 1:length(G)
        if is_linearly_independent(vcat(T,s), G[i], p)
            v=smallest_base(vcat(T, [G[i]]), p)
            if Number_of_multiplications(v,p)<Number_of_multiplications(T,p)
                s=vcat(s, [G[i]])
            else
                G_new=[j for j in G[i+1:end] if is_linearly_independent(vcat(v,s), j, p)]
                return filter(T,G_new,p,s)
            end
        end
    end
    return s
end
# Function to build up the base set from the reverse
# Recursive function to expand the subspace
function Has_one_rank_basis(T,G,p)
	s=[]
	for i in G
		if is_linearly_independent(s, i,p) && !is_linearly_independent(T, i,p)
			s=vcat(s,[i])
		end
	end
	return length(s)
end

function expand_subspace(W, G,j,k, p)
    HSK=Has_one_rank_basis(W,G, p)
    if HSK == k && length(W)==k
        return W
    end
    if length(W) < k
        for g in j:length(G)
            if is_linearly_independent(W, G[g],p)
				v=expand_subspace(vcat(W,[G[g]]), G,g+1, k, p)
				if v!== nothing
                	return v
				end
            end
        end
    end
    return nothing
end

# Main function
function main()
    print("Enter the prime number p: ")
    p = parse(Int, readline())
    println("Enter The Bilinear Matrix")
    input_text = readline()
    bilinear = eval( Meta.parse(input_text))
    println("\nOriginal bilinear map:")
    for matrix in bilinear
        display(matrix)
    end
    T= [Matrix{Int}(vcat(h...)) for h in bilinear]
    println("\nOriginal Number_of_multiplications")
    println(Number_of_multiplications(T,p))
    pseudo_solution1 = @time smallest_base(T,p)
    println("\nEtape 1")
    println(Number_of_multiplications(pseudo_solution1,p))
    println(pseudo_solution1)
    println("\nEtape 2")
    pseudo_solution2=@time rank_minimizer(pseudo_solution1,filter(pseudo_solution1,rank1_base(pseudo_solution1,p),p),p)
    println(Number_of_multiplications(pseudo_solution2,p))
    println(pseudo_solution2)
    println("\nEtape 3")
    println("\nGenerating G")
    n,m=size(T[1])
    G=generate_G(n,m,p)
    println("\nlenght of G ",length(G))
    optimized_G=filter(pseudo_solution1,G,p)
    println("\nlenght of optimized_G ",length(optimized_G))
    pseudo_solution3=@time rank_minimizer(pseudo_solution2,filter(pseudo_solution2,optimized_G,p),p)
    println(Number_of_multiplications(pseudo_solution3,p))
    println(pseudo_solution3)
    println("\nif u want to know if there is a rank equal to some value k, enter Y")
    input_text = readline()
    if input_text == "Y"
        print("Enter the value of k: ")
        k = parse(Int, readline())
        println("\nExpanding the subspace to rank $k")
        expanded_subspace = expand_subspace(pseudo_solution3, G, 1, k, p)
        if expanded_subspace == nothing
            println("No rank $k basis found")
        else
            println("Rank $k basis found:")
            for matrix in expanded_subspace
                display(matrix)
            end
        end
    end
end

# Run the main function
main()
#Enter:
#2
#[[[1 0 0], [0 0 0]],[[0 1 0], [1 0 0]],[[0 0 1], [0 1 0]],[[0 0 0], [0 0 1]]]

