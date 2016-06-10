/*
Copyright (c) 2016 
Javier Garcia Blas (fjblas@inf.uc3m.es)
Jose Daniel Garcia Sanchez (josedaniel.garcia@uc3m.es)
Yasser Aleman (yaleman@hggm.es)
Erick Canales (ejcanalesr@gmail.com)

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and 
associated documentation files (the "Software"), to deal in the Software without restriction, including 
without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell 
copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the 
following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial 
portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT 
LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN 
NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER 
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR 
THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef CREATE_KERNEL_RUMBA_H
#define CREATE_KERNEL_RUMBA_H


#include "create_signal_multi_tensor.hpp"

#include <plog/Log.h>
#include <iostream>
#include <math.h>
#include <armadillo>

namespace phardi {

  
    template <typename T>
    void Cart2Sph(T x, T y, T z, T & phi, T & theta)
    {
        double hypotxy;
        hypotxy = std::sqrt(std::pow(x,2) + std::pow(y,2));

        // compute elev
        theta = std::atan2(z, hypotxy);

        // compute az
        phi = std::atan2(y, x);
    }

    template <typename T>
    void create_Kernel_for_rumba(const arma::Mat<T> & V,
                                 const arma::Mat<T> & diffGrads,
                                 const arma::Col<T> & diffBvals,
                                 T lambda1,
                                 T lambda2,
                                 T lambda_csf,
                                 T lambda_gm,
                                 arma::Mat<T> & Kernel,
                                 const phardi::options opts) {


        using namespace arma;
        // add_rician_noise = 0;
        bool add_rician_noise = opts.add_noise;

        T SNR = 1;

        // [phi, theta] = cart2sph(V(:,1),V(:,2),V(:,3)); % set of directions
        Col<T> phi(V.n_rows);
        Col<T> theta(V.n_rows);

	#pragma omp parallel for
        for (int i = 0; i < V.n_rows; ++i) {
            Cart2Sph(V(i,0),V(i,1),V(i,2),phi(i),theta(i));
        }

	theta.transform( [](T val) { return (-val); } );

        //S0 = 1; % The Dictionary is created under the assumption S0 = 1;
        T S0 = 1;
        //fi = 1; % volume fraction
        Col<T> fi(1);
        fi(0) = 1;

        Col<T> S(diffGrads.n_rows);
        Mat<T> D(3,3);
        Mat<T> v2(fi.n_elem,2);
        Col<T> v3(3);

        T c = 180/M_PI;

        //for i=1:length(phi)
        for (size_t i = 0; i < phi.n_elem; i++) {
            // anglesFi = [phi(i), theta(i)]*(180/pi); % in degrees
            v2(0,0) = phi(i)*c; v2(0,1) = theta(i)*c;
            v3(0) = lambda1; v3(1) = lambda2;  v3(2) = lambda2;

            S.fill(0.0);
            // Kernel(:,i) = create_signal_multi_tensor(anglesFi, fi, [lambda1, lambda2, lambda2], diffBvals, diffGrads, S0, SNR, add_rician_noise);            
            create_signal_multi_tensor<T>(v2, fi, v3, diffBvals, diffGrads, S0, SNR, add_rician_noise, S, D);
            for (size_t j = 0; j<S.n_elem;++j) {
                Kernel(i,j) = S(j);
            }
        }

        S.fill(0.0);
        v2(0,0) = phi(0)*c; v2(0,1) = theta(0)*c;
        v3(0) = lambda_csf; v3(1) = lambda_csf;  v3(2) = lambda_csf;
        create_signal_multi_tensor<T>(v2, fi, v3, diffBvals, diffGrads, S0, SNR, add_rician_noise, S, D);

	#pragma omp parallel for
        for (size_t i = 0; i<S.n_elem; ++i) {
            Kernel(phi.n_elem  ,i) = S(i);
        }

        S.fill(0.0);
        v2(0,0) = phi(0)*c; v2(0,1) = theta(0)*c;
        v3(0) = lambda_gm; v3(1) = lambda_gm;  v3(2) = lambda_gm;
        create_signal_multi_tensor<T>(v2, fi, v3, diffBvals, diffGrads, S0, SNR, add_rician_noise, S, D);

	#pragma omp parallel for
        for (size_t i = 0; i<S.n_elem; ++i) {
            Kernel(phi.n_elem + 1 ,i) = S(i);
        }

        Kernel = Kernel.t();
    }
}

#endif
