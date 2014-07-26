/*
  -------------------------------------------------------------------
  
  Copyright (C) 2012-2014, Andrew W. Steiner
  
  This file is part of O2scl.
  
  O2scl is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.
  
  O2scl is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with O2scl. If not, see <http://www.gnu.org/licenses/>.

  -------------------------------------------------------------------
*/
#ifndef O2SCL_VECTOR_DERINT_H
#define O2SCL_VECTOR_DERINT_H

/** \file vector_derint.h
    \brief Derivatives of integrals of functions stored in vectors
    with implicit fixed-size grid

    These functions differentiate or integrate a function over a
    fixed-size grid specified in a vector. Derivatives and integrals
    are always specified without the factor defining the grid size
    (i.e. \f$ dx \f$), so the user must always multiply or divide the
    result by the grid size afterwards as necessary.

    The derivative functions will call the error handler if they are
    given empty vectors or vectors of length 1. The integration rules
    often expect a minimum number of points, so for smaller vectors
    they fall back onto rules which use fewer points. For empty
    vectors they return zero, for vectors of length 1, they always
    return the sole element of the vector, and for vectors of length
    2, they always return the average of the two elements. If the
    vector length is zero, the integration functions call the 
    error handler. 

    More points does not always mean higher accuracy. 
*/
#include <o2scl/interp.h>

#ifndef DOXYGEN_NO_O2NS
namespace o2scl {
#endif

  /** \name Compute the derivative at every point of a generic vector

      Given a vector \c v of size \c n, these functions compute
      the derivative at every point and store the result in \c dv.
   */
  //@{
  /** \brief Derivative of a vector with a three-point formula
   */
  template<class vec_t, class vec2_t> 
    void vector_deriv_threept(size_t n, vec_t &v, vec2_t &dv) {
    if (n<=1) {
      O2SCL_ERR2("Requested derivative of zero or one-element vector ",
		 "in vector_deriv_threept().",exc_einval);
    }
    if (n==2) {
      double d=v[1]-v[0];
      dv[0]=d;
      dv[1]=d;
      return;
    }
    dv[0]=-1.5*v[0]+2.0*v[1]-0.5*v[2];
    dv[n-1]=1.5*v[n-1]-2.0*v[n-2]+0.5*v[n-3];
    for(size_t i=1;i<n-1;i++) {
      dv[i]=(v[i+1]-v[i-1])/2.0;
    }
    return;
  }

  /** \brief Derivative of a vector with a three-point formula
      using two-point at the edges
  */
  template<class vec_t, class vec2_t> 
    void vector_deriv_threept_tap(size_t n, vec_t &v, vec2_t &dv) {
    if (n<=1) {
      O2SCL_ERR2("Requested derivative of zero or one-element vector ",
		 "in vector_deriv_threept().",exc_einval);
    }
    if (n==2) {
      double d=v[1]-v[0];
      dv[0]=d;
      dv[1]=d;
      return;
    }
    // 2-point
    dv[0]=v[1]-v[0];
    dv[n-1]=v[n-1]-v[n-2];
    // 3-point
    for(size_t i=1;i<n-1;i++) {
      dv[i]=(v[i+1]-v[i-1])/2.0;
    }
    return;
  }

  /** \brief Derivative of a vector with a five-point formula
   */
  template<class vec_t, class vec2_t> 
    void vector_deriv_fivept(size_t n, vec_t &v, vec2_t &dv) {
    if (n<=1) {
      O2SCL_ERR2("Requested derivative of zero or one-element vector ",
		 "in vector_deriv_fivept().",exc_einval);
    }
    if (n==2) {
      double d=v[1]-v[0];
      dv[0]=d;
      dv[1]=d;
      return;
    }
    dv[0]=-25.0/12.0*v[0]+4.0*v[1]-3.0*v[2]+4.0/3.0*v[3]-0.25*v[4];
    dv[1]=-0.25*v[0]-5.0/6.0*v[1]+1.5*v[2]-0.5*v[3]+v[4]/12.0;
    dv[n-2]=-v[n-5]/12.0+0.5*v[n-4]-1.5*v[n-3]+5.0/6.0*v[n-2]+0.25*v[n-1];
    dv[n-1]=0.25*v[n-5]-4.0*v[n-4]/3.0+3.0*v[n-3]-4.0*v[n-2]+25.0*v[n-1]/12.0;
    for(size_t i=2;i<n-2;i++) {
      dv[i]=1.0/12.0*(v[i-2]-v[i+2])+2.0/3.0*(v[i+1]-v[i-1]);
    }
    return;
  }

  /** \brief Derivative of a vector with a five-point formula with 
      four- and three-point formulas used at the edges
  */
  template<class vec_t, class vec2_t> 
    void vector_deriv_fivept_tap(size_t n, vec_t &v, vec2_t &dv) {
    if (n<=1) {
      O2SCL_ERR2("Requested derivative of zero or one-element vector ",
		 "in vector_deriv_fivept().",exc_einval);
    }
    if (n==2) {
      double d=v[1]-v[0];
      dv[0]=d;
      dv[1]=d;
      return;
    }
    // 3-point
    dv[0]=-1.5*v[0]+2.0*v[1]-0.5*v[2];
    dv[n-1]=1.5*v[n-1]-2.0*v[n-2]+0.5*v[n-3];
    // 4-point
    dv[1]=-v[0]/3.0-v[1]/2.0+v[2]-v[3]/6.0;
    dv[n-2]=v[n-4]/6.0-v[n-3]+v[n-2]/2.0+v[n-1]/3.0;
    // 5-point
    for(size_t i=2;i<n-2;i++) {
      dv[i]=1.0/12.0*(v[i-2]-v[i+2])+2.0/3.0*(v[i+1]-v[i-1]);
    }
    return;
  }
  
  /** \brief Derivative from interpolation object
   */
  template<class ovec_t, class vec2_t>
    void vector_deriv_interp(size_t n, ovec_t &v, vec2_t &dv, 
			     size_t interp_type=itp_cspline) {
    ovec_t grid(n);
    for(size_t i=0;i<n;i++) grid[i]=((double)i);
    interp_vec<ovec_t,ovec_t> oi(n,grid,v,interp_type);
    for(size_t i=0;i<n;i++) dv[i]=oi.deriv(((double)i));
    return;
  }

  /** \brief Derivative from interpolation object
   */
  template<class vec_t, class vec2_t, class vec3_t>
    void vector_deriv_interp(size_t n, vec_t &vx, vec2_t &vy, vec3_t &dv, 
			     size_t interp_type=itp_cspline) {
    interp_vec<vec_t,vec2_t> oi(n,vx,vy,interp_type);
    for(size_t i=0;i<n;i++) dv[i]=oi.deriv(vx[i]);
    return;
  }
  //@}

  /** \name Compute the integral over a generic vector

      These functions implement composite (sometimes called
      extended) Newton-Cotes rules. 

      Given a vector \c v of size \c n, these functions compute
      the integral over the entire vector and return the result.

      These functions were principally based on the information at
      http://mathworld.wolfram.com/Newton-CotesFormulas.html .
   */
  //@{
  /** \brief Integrate with an extended trapezoidal rule. 
   */
  template<class vec_t> double vector_integ_trap(size_t n, vec_t &v) {
    if (n==0) {
      O2SCL_ERR2("Tried to integrate zero-length vector in ",
		     "vector_integ_trap",exc_einval);
    } else if (n==1) {
      return v[0];
    }
    double res=(v[0]+v[n-1])/2.0;
    for(size_t i=1;i<n-1;i++) res+=v[i];
    return res;
  }

  /** \brief Integrate with an extended 3-point rule (extended 
      Simpson's rule)

      \note This function uses an untested hack I wrote for even n. 
  */
  template<class vec_t> double vector_integ_threept(size_t n, vec_t &v) {
    if (n==0) {
      O2SCL_ERR2("Tried to integrate zero-length vector in ",
		     "vector_integ_threept",exc_einval);
    } else if (n==1) {
      return v[0];
    } else if (n==2) {
      return (v[0]+v[1])/2.0;
    }
    double res=v[0]+v[n-1];
    // If true, next terms have a prefactor of 4, otherwise
    // the next terms have a prefactor of 2
    bool four=true;
    for(size_t i=1;i<n/2;i++) {
      // Correct if n is even
      if (i==n-i-2) {
	if (four) res+=(v[i]+v[n-i-1])*3.5;
	else res+=(v[i]+v[n-i-1])*2.5;
      } else {
	if (four) res+=(v[i]+v[n-i-1])*4.0;
	else res+=(v[i]+v[n-i-1])*2.0;
      }
      four=!four;
    }
    return res/3.0;
  }

  /** \brief Integrate with an extended rule for 4 or more points. 

      This function falls back to the equivalent of
      vector_integ_threept() for 3 points.
  */
  template<class vec_t> double vector_integ_extended4(size_t n, vec_t &v) {
    if (n==0) {
      O2SCL_ERR2("Tried to integrate zero-length vector in ",
		     "vector_integ_extended4",exc_einval);
    } else if (n==1) {
      return v[0];
    } else if (n==2) {
      return (v[0]+v[1])/2.0;
    } else if (n==3) {
      return (v[0]+4.0*v[1]+v[2])/3.0;
    }
    double res=(v[0]*5.0+v[1]*13.0+v[n-1]*5.0+v[n-2]*13.0)/12.0;
    for(size_t i=2;i<n-2;i++) {
      res+=v[i];
    }
    return res;
  }

  /** \brief Integrate with Durand's rule for 4 or more points. 

      This function falls back to the equivalent of
      vector_integ_threept() for 3 points.
  */
  template<class vec_t> double vector_integ_durand(size_t n, vec_t &v) {
    if (n==0) {
      O2SCL_ERR2("Tried to integrate zero-length vector in ",
		     "vector_integ_durand",exc_einval);
    } else if (n==1) {
      return v[0];
    } else if (n==2) {
      return (v[0]+v[1])/2.0;
    } else if (n==3) {
      return (v[0]+4.0*v[1]+v[2])/3.0;
    }
    double res=(v[0]*4.0+v[1]*11.0+v[n-1]*4.0+v[n-2]*11.0)/10.0;
    for(size_t i=2;i<n-2;i++) {
      res+=v[i];
    }
    return res;
  }

  /** \brief Integrate with an extended rule for 8 or more points. 

      This function falls back to vector_integ_extended4() 
      for less than 8 points.
  */
  template<class vec_t> double vector_integ_extended8(size_t n, vec_t &v) {
    if (n<8) return vector_integ_extended4(n,v);
    double res=((v[0]+v[n-1])*17.0+(v[1]+v[n-2])*59.0+(v[2]+v[n-3])*43.0+
		(v[3]+v[n-4])*49.0)/48.0;
    for(size_t i=4;i<n-4;i++) {
      res+=v[i];
    }
    return res;
  }

  /** \brief Integral from interpolation object
   */
  template<class ovec_t>
    double vector_integ_interp(size_t n, ovec_t &v, size_t interp_type) {
    ovec_t grid(n);
    for(size_t i=0;i<n;i++) grid[i]=((double)i);
    interp_vec<ovec_t> oi(n,grid,v,interp_type);
    return oi.integ(0.0,((double)(n-1)));
  }
  //@}

#ifndef DOXYGEN_NO_O2NS
}
#endif

#endif
