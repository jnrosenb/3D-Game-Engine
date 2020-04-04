// FFT.h
// -- Fast Discrete Fourier Transform
// jsh 10/09

#pragma once

#include <complex>
#include <cmath>

template<class R>
class FFT_base 
{
public:
    FFT_base(unsigned int lg_dim, bool inverse);
    ~FFT_base();
    
    template <class T>
    std::complex<R> *operator()(T *array);
  
private:
    FFT_base& operator=(FFT_base const& rhs);

private:
    //Log of number of cells
    const unsigned int logDim;

    //Number of cells
    const unsigned int dimension;
    
    //-FT  -->  2pi
    //-IFT --> -2pi
    R sign2pi;

    //Norm is the term that multiplies 
    //the summation when doing IDFT
    //-IF inv -->> norm = 1.0/dim
    //-ELSE   -->> norm = 1.0 
    R norm;
    
    //For FFT -> Index manipulation for handling merge
    unsigned int *bit_reverse;

    //Container in which the elements will be placed on
    //the correct order for doing Fast Fourier
    std::complex<R> *transform;
};


template<class R>
class FFT : public FFT_base<R>
{
public:
    FFT(unsigned lg_dim);
};


template<class R>
class IFFT : public FFT_base<R> 
{
public:
    IFFT(unsigned lg_dim);
};



////////////////////////////////////////////////
///         IMPLEMENTATIONS                 ////
////////////////////////////////////////////////

template<class R>
FFT_base<R>::FFT_base(unsigned logDim, bool inv) : 
    logDim(logDim), dimension(1 << logDim), 
    sign2pi( (inv ? R(8) : R(-8)) * std::atan( R(1) ) ),
    norm(inv ? R(1) / R(dimension) : R(1))
{
    transform = new std::complex<R>[dimension];
    bit_reverse = new unsigned[dimension];

    //Setup the indices for Fast Fourier
    for (unsigned i = 0; i < dimension; ++i) 
    {
        unsigned r = 0;    
        for (unsigned int j = 0, n = i; j < logDim; ++j, n >>= 1)
            r = (r << 1) | (n & 0x01);
        bit_reverse[i] = r;
    }
}


template<class R>
FFT_base<R>::~FFT_base() 
{
    delete[] transform;
    delete[] bit_reverse;
}


template<class R>       // R is real, float
template<class OTHER>   // Other will be the container with original complex<float> samples
std::complex<R> *FFT_base<R>::operator()(OTHER *in) 
{
    //Get input array on transform on the correct order for bottom up
    for (unsigned p = 0; p < dimension; ++p)
        transform[p] = std::complex<R>(in[bit_reverse[p]]);
    
    //Outer loop. LogN
    for (unsigned h = 0; h < logDim; ++h) 
    {
        unsigned m = (1 << h); //2^h -- Current level length
        unsigned n = (m << 1); //2*m -- Parent level length
        
        //e^{+-(2pi)/N} >> 2pi/N has the i implicit (img part on complex ctor)
        std::complex<R> twidle = std::exp(std::complex<R>(0, sign2pi/n));

        //Current level is m, of length n/2
        //k goes from 0 to dim, on increments of n (so it creates dim/n subsets of length n)
        //These are the subsets which, next iteration, will be the current level (of length m each)
        for (unsigned k = 0; k < dimension; k += n) 
        {
            //Starts as 1 (base level?)
            std::complex<R> twidle_j(1);
            
            //Fill each subset of length n with each half using FFT lemma
            for (unsigned j = 0; j < m; ++j) 
            {
                //e^{(-i)(2pi)(k)/N} * b_k
                std::complex<R> w = twidle_j * transform[k + j + m];
                std::complex<R> ak = transform[k + j];

                //Between Each iter, we are filling one 
                //position of each half of subset of length n
                transform[k + j]     = ak + w;    //ak + e*bk
                transform[k + j + m] = ak - w;    //ak - e*bk

                //e^{ (-i)(2pi)(k)/N} <-- previous * e^{-(2pi)/N}    
                twidle_j *= twidle;
            }
        }
    }
    
    //This is the term we multiply for the IFT, the 1/N
    for (unsigned p = 0; p < dimension; ++p)
        transform[p] *= norm;
    
    //After all is done, return vector of DFT
    return transform;
}


template<class R>
FFT<R>::FFT(unsigned lg_dim) : FFT_base<R>(lg_dim, false) 
{
}


template<class R>
IFFT<R>::IFFT(unsigned lg_dim) : FFT_base<R>(lg_dim, true) 
{
}
