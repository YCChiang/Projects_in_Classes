# Image Compression

Using singular value decomposition to compress the image.

## Background

### Singular Value and Singular Vector

Singular value and singular vector are satisfying *Av = su* while *v* is a right singular vector, *u* is a left singular vector. *s* is a singular value.

### Singular Value Decompsition [Wiki][7]

The left-singular vectors of *A* are a set of orthonormal eigenvectors.

The right-singular vectors of *A* are a set of orthonormal eigenvectors.

![Figure 1](Singular&#32;value&#32;decomposition.png)

### Eigenvalue and Eigenvector

Eigenvalue and eigenvectors are satisfying *Ax = λx*

### QR Algorithm

An algorithm to compute eigenvalues and eigenvectors. If the original matrix is symmetric and tridiagonal, we can accelerate the process of the computation. [Wiki][3]

### Lanczos Algorithm

An algorithm to tridiagonalize of a Hermitian matrix. As we now have computed for a real matrix(image matrix), a hermitian matrix is the same as a symmetric matrix. For Computation of Lanczos algorithm, see: [Wiki][5]

## Computation for Singular Value Decompsition

There are two different conditions:

- Matrix *A* is a symmetric matrix
- Matrix *A* is a nonsymmetric matrix

### The First Condition

First, we can find when *u* equals *v*, the singular values are the same as its eigenvalues, and all singular vectors are equivalent to its eigenvectors. So the SVD is equivalent to eigenvalue decomposition. And there are 3 methods for computing the SVD when the matrix is symmetric. See: [How to compute the SVD of a symmetric matrix?][1]

So we now focus on how to compute the eigenvectors and eigenvalues. As we mention in the background, the QR algorithm is a suitable method.

After we choose the QR algorithm to compute eigenvectors and eigenvalues, we find that an image its matrix *A* is a real matrix, so *A* is a [Hermitian matrix][4] as well. So we can tridiagnalize *A* to accelerate the computation.

### The Second Condition

What if our matrix *A* is not a symmetric matrix? There are many images that are not symmetric. How we compute the SVD? [YouTube video recommendation][6]

## Compress the Image

Now, we got the result of SVD for the image. How to compress it?

We retain some singular values, as the figure shows below k singular values are retained and discarded other singular values, those smaller ones.

The comprssion ratio:

![Figure 2](Compression&#32;ratio.png)

If we choose k equals to n and multiple all the result matrix as to how we decompose it, we can get the image back without compression.

[1]: https://math.stackexchange.com/questions/22825/how-to-compute-the-svd-of-a-symmetric-matrix
[2]: https://en.wikipedia.org/wiki/Singular_value_decomposition
[3]: https://en.wikipedia.org/wiki/QR_algorithm
[4]: https://en.wikipedia.org/wiki/Hermitian_matrix
[5]: https://en.wikipedia.org/wiki/Lanczos_algorithm#The_algorithm
[6]: https://www.youtube.com/watch?v=Ls2TgGFfZnU
[7]: https://en.wikipedia.org/wiki/Singular_value_decomposition