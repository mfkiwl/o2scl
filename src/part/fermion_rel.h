/* -------------------------------------------------------------------
  
   Copyright (C) 2006-2021, Andrew W. Steiner
  
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
#ifndef O2SCL_FERMION_REL_H
#define O2SCL_FERMION_REL_H

/** \file fermion_rel.h
    \brief File defining \ref o2scl::fermion_rel_tl
*/

#include <string>
#include <iostream>
#include <fstream>
#include <cmath>

#ifdef O2SCL_LD_TYPES
#include <boost/multiprecision/cpp_dec_float.hpp>
#endif

#include <o2scl/constants.h>
#include <o2scl/mroot.h>
#include <o2scl/inte.h>
#include <o2scl/fermion.h>
#include <o2scl/root_brent_gsl.h>
#include <o2scl/inte_qagiu_gsl.h>
#include <o2scl/inte_qag_gsl.h>
#include <o2scl/polylog.h>

#ifndef DOXYGEN_NO_O2NS
namespace o2scl {
#endif

  /** \brief Equation of state for a relativistic fermion

      This class computes the thermodynamics of a relativistic fermion
      either as a function of the density or the chemical potential.
      It employs direct integration, using two different integrators
      for the degenerate and non-degenerate regimes. The default
      integrators are inte_qag_gsl (for degenerate fermions) and
      inte_qagiu_gsl (for non-degenerate fermions). For the functions
      calc_mu() and calc_density(), if the temperature argument is
      less than or equal to zero, the functions \ref
      fermion_zerot::calc_mu_zerot() and \ref
      fermion_zerot::calc_density_zerot() will be used to compute the
      result.

      \hline 
      <b>Degeneracy parameter:</b>

      Define the degeneracy parameter 
      \f[
      \psi=(\nu-m^{*})/T 
      \f] 
      where \f$ \nu \f$ is the effective chemical potential (including
      the rest mass) and \f$
      m^{*} \f$ is the effective mass. For \f$ \psi \f$ smaller than
      \ref min_psi, the non-degenerate expansion in \ref
      fermion_thermo::calc_mu_ndeg() is attempted first. If that
      fails, then integration is used. For \f$ \psi \f$ greater than
      \ref deg_limit (degenerate regime), a finite interval integrator
      is used and for \f$ \psi \f$ less than \ref deg_limit
      (non-degenerate regime), an integrator over the interval from
      \f$ [0,\infty) \f$ is used. In the case where \ref
      part::inc_rest_mass is false, the degeneracy parameter is
      \f[
      \psi=(\nu+m-m^{*})/T 
      \f] 

      <b>Integration limits:</b>

      The upper limit on the degenerate integration is given by
      \f[
      \mathrm{upper~limit} = \sqrt{{\cal L}^2-m^{*,2}}
      \f]
      where \f$ {\cal L}\equiv u T+\nu \f$ and \f$ u \f$ is \ref
      fermion_rel::upper_limit_fac . In the case where \ref
      part::inc_rest_mass is false, the result is
      \f[
      \mathrm{upper~limit} = \sqrt{(m+{\cal L})^2-m^{*2}}
      \f]
      
      The entropy is only significant at the Fermi surface, thus
      in the degenerate case, the lower limit of the entropy
      integral can be given be determined by the value of \f$ k \f$ 
      which solves
      \f[
      - u = \frac{\sqrt{k^2+m^{* 2}}-\nu}{T}
      \f]
      The solution is 
      \f[
      \mathrm{lower~limit} = \sqrt{(-u T+{\nu})^2-m^{*,2}}
      \f]
      but this solution is only valid if \f$ (m^{*}-\nu)/T < -u \f$.
      In the case where part::inc_rest_mass is false, the result is
      \f[
      \mathrm{lower~limit} = \sqrt{(-u T + m +\nu)^2-m^{*,2}}
      \f]
      which is valid if \f$ (m^{*}-\nu - m)/T < -u \f$.

      <b>Entropy integrand:</b>

      In the degenerate regime, the entropy integrand
      \f[
      - k^2 \left[ f \log f + \left(1-f\right) \log 
      \left(1-f \right) \right]
      \f]
      where \f$ f \f$ is the fermionic distribution function can lose
      precision when \f$ (E^{*} - \nu)/T \f$ is negative and
      sufficiently large in absolute magnitude. Thus when \f$ (E^{*} -
      \nu)/T < S \f$ where \f$ S \f$ is stored in \ref deg_entropy_fac
      (default is -30), the integrand is written as
      \f[
      -k^2 \left( E/T-\nu/T \right) e^{E/T-\nu/T} \, .
      \f]
      If \f$ (E - \nu)/T < S \f$ is less than -1 times \ref exp_limit
      (e.g. less than -200), then the entropy integrand is assumed 
      to be zero.
      
      <b>Non-degenerate integrands:</b>
      
      \comment
      It's not at all clear that this dimensionless form is more
      accurate than other potential alternatives. On the other hand,
      it seems that the uncertainties in the integrations are larger
      than the errors made by the integrand at present.
      \endcomment
      The integrands in the non-degenerate regime are written
      in a dimensionless form, by defining \f$ u \f$ with
      the relation
      \f$ p = \sqrt{\left(T u + m^{*}\right)^2-m^{* 2}} \f$,
      \f$ y \equiv \nu/ T \f$, and 
      \f$ \eta \equiv m^{*}/T \f$. Then, 
      \f$ p/T=\sqrt{u^2+2 \eta u} \f$
      \f$ E/T = \mathrm{mx+u} \f$ and 
      \f$ p/T^2 dp = 2(\eta+u) du \f$
      The density integrand is 
      \f[
      \left(\eta+u\right) \sqrt{u^2+2 (\eta) u}
      \left(\frac{e^{y}}{e^{\eta+u}+e^{y}}\right) \, , 
      \f]
      the energy integrand is 
      \f[
      \left(\eta+u\right)^2 \sqrt{u^2+2 (\eta) u}
      \left(\frac{e^{y}}{e^{\eta+u}+e^{y}}\right) \, ,
      \f]
      and the entropy integrand is 
      \f[
      \left(\eta+u\right) \sqrt{u^2+2 (\eta) u} 
      \left(t_1+t_2\right) \, ,
      \f]
      where 
      \f{eqnarray*}
      t_1 &=& \log \left(1+e^{y-\eta-u}\right)/
      \left(1+e^{y-\eta-u}\right) \nonumber \\
      t_2 &=& \log \left(1+e^{\eta+u-y}\right)/
      \left(1+e^{\eta+u-y}\right) \, .
      \f}

      \hline 
      <b>Accuracy:</b>

      The default settings for for this class give an accuracy of at
      least 1 part in \f$ 10^6 \f$ (and frequently better than this).

      When the integrators provide numerical uncertainties, these
      uncertainties are stored in \ref unc. In the case of
      calc_density() and pair_density(), the uncertainty from the
      numerical accuracy of the solver is not included. (There is also
      a relatively small inaccuracy due to the mathematical evaluation
      of the integrands which is not included in \ref unc.)
     
      One can improve the accuracy to within 1 part in \f$ 10^{10} \f$ 
      using
      \code
      fermion_rel rf(1.0,2.0);
      rf.upper_limit_fac=40.0;
      rf.dit->tol_abs=1.0e-13;
      rf.dit->tol_rel=1.0e-13;
      rf.nit->tol_abs=1.0e-13;
      rf.nit->tol_rel=1.0e-13;
      rf.density_root->tol_rel=1.0e-10;
      \endcode
      which decreases the both the relative and absolute tolerances
      for both the degenerate and non-degenerate integrators and
      improves the accuracy of the solver which determines the
      chemical potential from the density. Of course if these
      tolerances are too small, the calculation may fail. 

      \hline 
      <b>Todos:</b>

      \future I had to remove the shared_ptr stuff because
      the default algorithm types don't support multiprecision,
      but it might be nice to restore the shared_ptr mechanism
      somehow.

      \future The expressions which appear in in the integrand
      functions density_fun(), etc. could likely be improved,
      especially in the case where \ref o2scl::part::inc_rest_mass is
      <tt>false</tt>. There should not be a need to check if
      <tt>ret</tt> is finite.

      \future It appears this class doesn't compute the uncertainty in
      the chemical potential or density with calc_density(). This
      could be fixed.

      \future I'd like to change the lower limit on the entropy 
      integration, but the value in the code at the moment (stored
      in <tt>ll</tt>) makes bm_part2.cpp worse.

      \future The function pair_mu() should set the antiparticle
      integrators as done in fermion_deriv_rel.
  */
  template<class fermion_t=fermion_tl<double>,
	   class fd_inte_t=class o2scl::fermi_dirac_integ_gsl,
	   class be_inte_t=o2scl::bessel_K_exp_integ_gsl,
	   class nit_t=inte_qagiu_gsl<>,
	   class dit_t=inte_qag_gsl<>,
	   class density_root_t=root_cern<>,
	   class root_t=root_cern<>, class func_t=funct,
	   class fp_t=double>
  class fermion_rel_tl :
    public fermion_thermo_tl<fermion_t,fd_inte_t,be_inte_t,root_t,
			     func_t,fp_t> {
    
  public:

    /// \name Numerical parameters
    //@{
    /** \brief If true, call the error handler when convergence 
	fails (default true)
    */
    bool err_nonconv;
    
    /** \brief The smallest value of \f$ (\mu-m)/T \f$ for which 
	integration is used (default -4.0)
    */
    fp_t min_psi;

    /** \brief The critical degeneracy at which to switch integration 
	techniques (default 2.0)
    */
    fp_t deg_limit;
    
    /** \brief The limit for exponentials to ensure integrals are finite 
	(default 200.0)
    */
    fp_t exp_limit;

    /// The factor for the degenerate upper limits (default 20.0)
    fp_t upper_limit_fac;

    /// A factor for the degenerate entropy integration (default 30.0)
    fp_t deg_entropy_fac;

    /// Verbosity parameter (default 0)
    int verbose;

    /// If true, use expansions for extreme conditions (default true)
    bool use_expansions;

    /// Tolerance for expansions (default \f$ 10^{-14} \f$)
    fp_t tol_expan;

    /// If true, verify the thermodynamic identity (default false)
    bool verify_ti;
    
    /// Value for verifying the thermodynamic identity
    fp_t therm_ident;
    //@}

    /// Storage for the uncertainty
    fermion_t unc;

    /// Create a fermion with mass \c m and degeneracy \c g
    fermion_rel_tl() {
      
      deg_limit=2.0;
      
      exp_limit=200.0;
      upper_limit_fac=20.0;
      deg_entropy_fac=30.0;
      min_psi=-4.0;
      err_nonconv=true;
      use_expansions=true;
      verbose=0;
      last_method=0;

      dit=&def_dit;
      nit=&def_nit;
      density_root=&def_density_root;
      
      density_root->tol_rel=4.0e-7;

      tol_expan=1.0e-14;
      verify_ti=false;
      therm_ident=0.0;
    }

    virtual ~fermion_rel_tl() {
    }

    /// Pointer to the non-degenerate integrator
    inte<func_t,fp_t> *nit;

    /// Pointer to the degenerate integrator
    inte<func_t,fp_t> *dit;

    /// The solver for calc_density()
    root<func_t,func_t,fp_t> *density_root;

    /// The default non-degenerate integrator
    nit_t def_nit;
    
    /// The default degenerate integrator
    dit_t def_dit;

    /// The default solver for the chemical potential given the density
    density_root_t def_density_root;
    
    /// Return string denoting type ("fermion_rel")
    virtual const char *type() { return "fermion_rel"; }

    /** \brief An integer indicating the last numerical method used

	In all functions
	- 0: no previous calculation or last calculation failed

	In \ref nu_from_n():
	- 1: default solver
	- 2: default solver with smaller tolerances
	- 3: bracketing solver

	In \ref calc_mu():
	- 4: non-degenerate expansion
	- 5: degenerate expansion
	- 6: exact integration, non-degenerate integrands
	- 7: exact integration, degenerate integrands, lower limit
	on entropy integration
	- 8: exact integration, degenerate integrands, full
	entropy integration
	- 9: T=0 result

	In \ref calc_density(), the integer is a two-digit
	number. The first digit (1 to 3) is the method used by \ref
	nu_from_n() and the second digit is one of
	- 1: nondegenerate expansion
	- 2: degenerate expansion
	- 3: exact integration, non-degenerate integrands
	- 4: exact integration, degenerate integrands, lower limit
	on entropy integration
	- 5: exact integration, degenerate integrands, full
	entropy integration
	If \ref calc_density() uses the T=0 code, then
	last_method is 40. 

	In \ref pair_mu(), the integer is a three-digit number.
	The third digit is always 0 (to ensure a value of last_method
	which is unique from the other values reported from other
	functions as described above). The first digit is the method
	used for particles from \ref calc_mu() above and the
	second digit is the method used for antiparticles. 

	In \ref pair_density(), the integer is a four-digit
	number. The first digit is from the list below and the
	remaining three digits, if nonzero, are from \ref
	pair_mu().
	- 1: T=0 result
	- 2: default solver
	- 3: bracketing solver
	- 4: default solver with smaller tolerances
	- 5: default solver with smaller tolerances in log units
	- 6: bracketing in log units
    */
    int last_method;

    /// \name Template versions of base functions
    //@{
    /** \brief Calculate the chemical potential from the density
	(template version)
    */
    int nu_from_n(fermion_t &f, fp_t temper) {

      last_method=0;
      
      fp_t nex;

      // Try to ensure a good initial guess

      nex=f.nu/temper;
      fp_t y=solve_fun(nex,f,temper);
      if (verbose>1) {
	std::cout << "fermion_rel::nu_from_n(): " 
		  << "initial guess " << nex << std::endl;
      }

      if (y>1.0-1.0e-6) {
	fp_t scale=f.ms;
	if (temper>scale) scale=temper;
	for(size_t i=0;i<10;i++) {
	  if (nex<0.0) nex+=scale*1.0e5;
	  else nex*=10.0;
	  y=solve_fun(nex,f,temper);
	  if (y<1.0-1.0e-6) i=10;
	}
	if (verbose>1) {
	  std::cout << "fermion_rel::nu_from_n(): "
		    << "adjusted guess to " << nex << std::endl;
	}
      }

      // If that didn't work, try a different guess
      if (y>1.0-1.0e-6) {
	if (f.inc_rest_mass) {
	  nex=f.ms/temper;
	} else {
	  nex=(f.ms-f.m)/temper;
	}
	y=solve_fun(nex,f,temper);
	if (verbose>1) {
	  std::cout << "fermion_rel::nu_from_n(): "
		    << "adjusted guess (try 2) to "
		    << nex << std::endl;
	}
      }
  
      // If neither worked, call the error handler
      if (y==1.0 || !o2isfinite(y)) {
	O2SCL_CONV2_RET("Couldn't find reasonable initial guess in ",
			"fermion_rel::nu_from_n().",exc_einval,
			this->err_nonconv);
      }

      // Perform full solution
      func_t mf=std::bind(std::mem_fn<fp_t(fp_t,fermion_t &,fp_t)>
			  (&fermion_rel_tl<fermion_t,fd_inte_t,be_inte_t,
			   nit_t,dit_t,density_root_t,
			   root_t,func_t,fp_t>::solve_fun),
			  this,std::placeholders::_1,std::ref(f),temper);

      // The default o2scl::root object is of type root_cern,
      // and this solver has problems when the root is near 0.
      // Below, we switch to a root_brent_gsl object in the case
      // that the default solver fails.
  
      bool drec=density_root->err_nonconv;
      density_root->err_nonconv=false;
      int ret=density_root->solve(nex,mf);

      if (ret!=0) {
    
	if (verbose>1) {
	  std::cout << "nu_from_n(): density_root failed x="
		    << nex << " ." << std::endl;
	  std::cout << "\tTrying to make integrators more accurate."
		    << std::endl;
	}

	// If it fails, try to make the integrators more accurate
	fp_t tol1=dit->tol_rel, tol2=dit->tol_abs;
	fp_t tol3=nit->tol_rel, tol4=nit->tol_abs;
	dit->tol_rel/=1.0e2;
	dit->tol_abs/=1.0e2;
	nit->tol_rel/=1.0e2;
	nit->tol_abs/=1.0e2;
	ret=density_root->solve(nex,mf);

	if (ret!=0) {

	  if (verbose>1) {
	    std::cout << "nu_from_n(): density_root failed again x=" << nex
		      << " ." << std::endl;
	    std::cout << "Trying to bracket root." << std::endl;
	  }
	  
	  // (std::max doesn't work with boost::multiprecision?)
	  fp_t lg;
	  if (o2abs(f.nu)>f.ms) lg=o2abs(f.nu);
	  lg=f.ms;
	  
	  fp_t bhigh=lg/temper, b_low=-bhigh;
	  fp_t yhigh=mf(bhigh), ylow=mf(b_low);
	  for(size_t j=0;j<5 && yhigh>0.0;j++) {
	    bhigh*=1.0e2;
	    yhigh=mf(bhigh);
	  }
	  for(size_t j=0;j<5 && ylow<0.0;j++) {
	    b_low*=1.0e2;
	    ylow=mf(b_low);
	  }
	  if (yhigh<0.0 && ylow>0.0) {
	    o2scl::root_brent_gsl<func_t,fp_t> rbg;
	    rbg.err_nonconv=false;
	    ret=rbg.solve_bkt(b_low,bhigh,mf);
	    if (ret==0) {
	      // Bracketing solver worked
	      last_method=3;
	      nex=b_low;
	    } else {
	      if (verbose>1) {
		std::cout << "nu_from_n(): density_root failed fourth solver "
			  << b_low << std::endl;
	      }
	    }
	  } else if (verbose>1) {
	    std::cout << "nu_from_n(): Failed to bracket." << std::endl;
	  }
	} else {
	  // Increasing tolerances worked
	  last_method=2;
	}

	// Return tolerances to their original values
	dit->tol_rel=tol1;
	dit->tol_abs=tol2;
	nit->tol_rel=tol3;
	nit->tol_abs=tol4;

      } else {
	// First solver worked
	last_method=1;
      }

      density_root->err_nonconv=drec;

      if (ret!=0) {
	O2SCL_CONV2_RET("Density solver failed in ",
			"fermion_rel::nu_from_n().",exc_efailed,
			this->err_nonconv);
      }

      f.nu=nex*temper;

      return success;
    }

    /** \brief Calculate properties as function of chemical potential
     */
    void calc_mu(fermion_t &f, fp_t temper) {

      if (verbose>1) {
	std::cout << "calc_mu(): start."
		  << std::endl;
      }
      
      last_method=0;
      
      // -----------------------------------------------------------------
      // Handle T<=0

      if (temper<0.0) {
	O2SCL_ERR2("Temperature less than zero in ",
		   "fermion_rel::calc_mu().",exc_einval);
      }
      if (temper==0.0) {
	this->calc_mu_zerot(f);
	last_method=9;
	return;
      }

      if (f.non_interacting==true) { f.nu=f.mu; f.ms=f.m; }

      // Compute the degeneracy parameter
  
      bool deg=true;
      fp_t psi;
      if (f.inc_rest_mass) {
	psi=(f.nu-f.ms)/temper;
      } else {
	psi=(f.nu+(f.m-f.ms))/temper;
      }
      if (psi<deg_limit) deg=false;
      
      if (verbose>1) {
	std::cout << "calc_mu(): psi,deg,deg_limit: " << psi << " "
		  << deg << " " << deg_limit << std::endl;
      }
      // Try the non-degenerate expansion if psi is small enough
      if (use_expansions && psi<min_psi) {
	bool acc=this->calc_mu_ndeg(f,temper,tol_expan);
	if (verbose>1) {
	  std::cout << "calc_mu(): non-deg expan " << acc
		    << std::endl;
	}
	if (acc) {
	  unc.n=f.n*tol_expan;
	  unc.ed=f.ed*tol_expan;
	  unc.pr=f.pr*tol_expan;
	  unc.en=f.en*tol_expan;
	  last_method=4;
	  return;
	}
      }

      // Try the degenerate expansion if psi is large enough
      if (use_expansions && psi>20.0) {
	bool acc=this->calc_mu_deg(f,temper,tol_expan);
	if (verbose>1) {
	  std::cout << "calc_mu(): deg expan " << acc
		    << std::endl;
	}
	if (acc) {
	  unc.n=f.n*tol_expan;
	  unc.ed=f.ed*tol_expan;
	  unc.pr=f.pr*tol_expan;
	  unc.en=f.en*tol_expan;
	  last_method=5;
	  return;
	}
      }

      if (!deg) {
    
	// If the temperature is large enough, perform the full integral
    
	func_t mfd=std::bind(std::mem_fn<fp_t(fp_t,fermion_t &,fp_t)>
			     (&fermion_rel_tl<fermion_t,fd_inte_t,be_inte_t,
			      nit_t,dit_t,density_root_t,
			      root_t,func_t,fp_t>::density_fun),
			     this,std::placeholders::_1,std::ref(f),temper);
	func_t mfe=std::bind(std::mem_fn<fp_t(fp_t,fermion_t &,fp_t)>
			     (&fermion_rel_tl<fermion_t,fd_inte_t,be_inte_t,
			      nit_t,dit_t,density_root_t,
			      root_t,func_t,fp_t>::energy_fun),
			     this,std::placeholders::_1,std::ref(f),temper);
	func_t mfs=std::bind(std::mem_fn<fp_t(fp_t,fermion_t &,fp_t)>
			     (&fermion_rel_tl<fermion_t,fd_inte_t,be_inte_t,
			      nit_t,dit_t,density_root_t,
			      root_t,func_t,fp_t>::entropy_fun),
			     this,std::placeholders::_1,std::ref(f),temper);
      
	fp_t prefac=f.g*pow(temper,3.0)/2.0/this->pi2;

	// Compute the number density
    
	if (verbose>1) {
	  std::cout << "calc_mu(): non-deg number density:"
		    << std::endl;
	}
	
	f.n=nit->integ_iu(mfd,0.0);
	f.n*=prefac;
	unc.n=nit->get_error()*prefac;

	// Compute the energy density

	if (verbose>1) {
	  std::cout << "calc_mu(): non-deg energy density:"
		    << std::endl;
	}
	
	f.ed=nit->integ_iu(mfe,0.0);
	f.ed*=prefac*temper;
	if (!f.inc_rest_mass) f.ed-=f.n*f.m;
	unc.ed=nit->get_error()*prefac*temper;
    
	// Compute the entropy

	if (verbose>1) {
	  std::cout << "calc_mu(): non-deg entropy:"
		    << std::endl;
	}
	
	f.en=nit->integ_iu(mfs,0.0);
	f.en*=prefac;
	unc.en=nit->get_error()*prefac;

	if (verbose>1) {
	  std::cout << "calc_mu(): non-deg integrals done."
		    << std::endl;
	}
	
	last_method=6;

      } else {
    
	// Otherwise, apply a degenerate approximation, by making the
	// upper integration limit finite
    
	func_t mfd=std::bind(std::mem_fn<fp_t(fp_t,fermion_t &,fp_t)>
			     (&fermion_rel_tl<fermion_t,fd_inte_t,be_inte_t,
			      nit_t,dit_t,density_root_t,
			      root_t,func_t,fp_t>::deg_density_fun),
			     this,std::placeholders::_1,std::ref(f),temper);
	func_t mfe=std::bind(std::mem_fn<fp_t(fp_t,fermion_t &,fp_t)>
			     (&fermion_rel_tl<fermion_t,fd_inte_t,be_inte_t,
			      nit_t,dit_t,density_root_t,
			      root_t,func_t,fp_t>::deg_energy_fun),
			     this,std::placeholders::_1,std::ref(f),temper);
	func_t mfs=std::bind(std::mem_fn<fp_t(fp_t,fermion_t &,fp_t)>
			     (&fermion_rel_tl<fermion_t,fd_inte_t,be_inte_t,
			      nit_t,dit_t,density_root_t,
			      root_t,func_t,fp_t>::deg_entropy_fun),
			     this,std::placeholders::_1,std::ref(f),temper);

	fp_t prefac=f.g/2.0/this->pi2;
    
	// Compute the upper limit for degenerate integrals

	if (verbose>1) {
	  std::cout << "calc_mu(): deg integrals."
		    << std::endl;
	}
	
	fp_t arg;
	if (f.inc_rest_mass) {
	  arg=pow(upper_limit_fac*temper+f.nu,2.0)-f.ms*f.ms;
	} else {
	  arg=pow(upper_limit_fac*temper+f.nu+f.m,2.0)-f.ms*f.ms;
	}
	fp_t ul;
	if (arg>0.0) {
	  ul=sqrt(arg);
	} else {
	  f.n=0.0;
	  f.ed=0.0;
	  f.pr=0.0;
	  f.en=0.0;
	  unc.n=0.0;
	  unc.ed=0.0;
	  unc.pr=0.0;
	  unc.en=0.0;
	  O2SCL_ERR2("Zero density in degenerate limit in fermion_rel::",
		     "calc_mu(). Variable deg_limit set improperly?",
		     exc_efailed);
	  return;
	}
    
	// Compute the number density

	f.n=dit->integ(mfd,0.0,ul);
	f.n*=prefac;
	unc.n=dit->get_error()*prefac;
    
	// Compute the energy density

	f.ed=dit->integ(mfe,0.0,ul);
	f.ed*=prefac;
	unc.ed=dit->get_error()*prefac;

	// Compute the lower limit for the entropy integration

	fp_t ll;
	if (f.inc_rest_mass) {
	  arg=pow(-upper_limit_fac*temper+f.nu,2.0)-f.ms*f.ms;
	  if (arg>0.0 && (f.ms-f.nu)/temper<-upper_limit_fac) {
	    ll=sqrt(arg);
	  } else {
	    ll=-1.0;
	  }
	} else {
	  arg=pow(-upper_limit_fac*temper+f.nu+f.m,2.0)-f.ms*f.ms;
	  if (arg>0.0 && (f.ms-f.nu-f.m)/temper<-upper_limit_fac) {
	    ll=sqrt(arg);
	  } else {
	    ll=-1.0;
	  }
	}

	// Compute the entropy

	if (ll>0.0) {
	  f.en=dit->integ(mfs,ll,ul);
	  last_method=7;
	} else {
	  f.en=dit->integ(mfs,0.0,ul);
	  last_method=8;
	}
	f.en*=prefac;
	unc.en=dit->get_error()*prefac;

	if (verbose>1) {
	  std::cout << "calc_mu(): deg integrals done."
		    << std::endl;
	}
	
	
      }

      // Compute the pressure using the thermodynamic identity

      f.pr=-f.ed+temper*f.en+f.nu*f.n;
      unc.pr=sqrt(unc.ed*unc.ed+temper*unc.en*temper*unc.en+
		  f.nu*unc.n*f.nu*unc.n);

      return;
    }

    /** \brief Calculate properties as function of density

	This function uses the current value of \c nu (or \c mu if the
	particle is non interacting) for an initial guess to solve for
	the chemical potential. If this guess is too small, then this
	function may fail.

	\future There is still quite a bit of code duplication
	between this function and \ref calc_mu() .
    */
    int calc_density(fermion_t &f, fp_t temper) {

      last_method=0;
      
      // The code below may modify the density, which is confusing to
      // the user, so we store it here so we can restore it before
      // this function returns
      fp_t density_temp=f.n;
  
      // -----------------------------------------------------------------
      // Handle T<=0

      if (temper<0.0) {
	O2SCL_ERR2("Temperature less than zero in ",
		   "fermion_rel::calc_density().",
		   exc_einval);
      }
      if (temper==0.0) {
	last_method=40;
	this->calc_density_zerot(f);
	return 0;
      }

#if !O2SCL_NO_RANGE_CHECK
      // This may not be strictly necessary, because it should be clear
      // that this function will produce gibberish if the density is not
      // finite, but I've found this extra checking of the inputs useful
      // for debugging.
      if (!o2isfinite(f.n)) {
	O2SCL_ERR2("Density not finite in ",
		   "fermion_rel::calc_density().",exc_einval);
      }
#endif

      // -----------------------------------------------------------------
      // First determine the chemical potential by solving for the density

      if (f.non_interacting==true) { f.nu=f.mu; f.ms=f.m; }
  
      int ret=nu_from_n(f,temper);
      last_method*=10;
      if (ret!=0) {
	O2SCL_CONV2_RET("Function calc_density() failed in fermion_rel::",
			"calc_density().",exc_efailed,this->err_nonconv);
      }

      if (f.non_interacting) { f.mu=f.nu; }

      // -----------------------------------------------------------------
      // Now use the chemical potential to compute the energy density,
      // pressure, and entropy

      bool deg=true;
      fp_t psi;
      if (f.inc_rest_mass) {
	psi=(f.nu-f.ms)/temper;
      } else {
	psi=(f.nu+(f.m-f.ms))/temper;
      }
      if (psi<deg_limit) deg=false;

      // Try the non-degenerate expansion if psi is small enough
      if (use_expansions && psi<min_psi) {
	bool acc=this->calc_mu_ndeg(f,temper,tol_expan);
	if (acc) {
	  unc.ed=f.ed*tol_expan;
	  unc.pr=f.pr*tol_expan;
	  unc.en=f.en*tol_expan;
	  f.n=density_temp;
	  last_method+=1;
	  return 0;
	}
      }
  
      // Try the degenerate expansion if psi is large enough
      if (use_expansions && psi>20.0) {
	bool acc=this->calc_mu_deg(f,temper,tol_expan);
	if (acc) {
	  unc.n=f.n*tol_expan;
	  unc.ed=f.ed*tol_expan;
	  unc.pr=f.pr*tol_expan;
	  unc.en=f.en*tol_expan;
	  f.n=density_temp;
	  last_method+=2;
	  return 0;
	}
      }

      if (!deg) {
    
	func_t mfe=std::bind(std::mem_fn<fp_t(fp_t,fermion_t &,fp_t)>
			     (&fermion_rel_tl<fermion_t,fd_inte_t,be_inte_t,
			      nit_t,dit_t,density_root_t,
			      root_t,func_t,fp_t>::energy_fun),
			     this,std::placeholders::_1,std::ref(f),temper);
	func_t mfs=std::bind(std::mem_fn<fp_t(fp_t,fermion_t &,fp_t)>
			     (&fermion_rel_tl<fermion_t,fd_inte_t,be_inte_t,
			      nit_t,dit_t,density_root_t,
			      root_t,func_t,fp_t>::entropy_fun),
			     this,std::placeholders::_1,std::ref(f),temper);
	
	f.ed=nit->integ_iu(mfe,0.0);
	f.ed*=f.g*pow(temper,4.0)/2.0/this->pi2;
	if (!f.inc_rest_mass) f.ed-=f.n*f.m;
	unc.ed=nit->get_error()*f.g*pow(temper,4.0)/2.0/this->pi2;
	
	f.en=nit->integ_iu(mfs,0.0);
	f.en*=f.g*pow(temper,3.0)/2.0/this->pi2;
	unc.en=nit->get_error()*f.g*pow(temper,3.0)/2.0/this->pi2;
	last_method+=3;

      } else {

	func_t mfe=std::bind(std::mem_fn<fp_t(fp_t,fermion_t &,fp_t)>
			     (&fermion_rel_tl<fermion_t,fd_inte_t,be_inte_t,
			      nit_t,dit_t,density_root_t,
			      root_t,func_t,fp_t>::deg_energy_fun),
			     this,std::placeholders::_1,std::ref(f),temper);
	func_t mfs=std::bind(std::mem_fn<fp_t(fp_t,fermion_t &,fp_t)>
			     (&fermion_rel_tl<fermion_t,fd_inte_t,be_inte_t,
			      nit_t,dit_t,density_root_t,
			      root_t,func_t,fp_t>::deg_entropy_fun),
			     this,std::placeholders::_1,std::ref(f),temper);
      
	fp_t arg;
	if (f.inc_rest_mass) {
	  arg=pow(upper_limit_fac*temper+f.nu,2.0)-f.ms*f.ms;
	} else {
	  arg=pow(upper_limit_fac*temper+f.nu+f.m,2.0)-f.ms*f.ms;
	}
	fp_t ul;
	if (arg>0.0) {
      
	  ul=sqrt(arg);
      
	  fp_t ll;
	  if (f.inc_rest_mass) {
	    arg=pow(-upper_limit_fac*temper+f.nu,2.0)-f.ms*f.ms;
	    if (arg>0.0 && (f.ms-f.nu)/temper<-upper_limit_fac) {
	      ll=sqrt(arg);
	    } else {
	      ll=-1.0;
	    }
	  } else {
	    arg=pow(-upper_limit_fac*temper+f.nu+f.m,2.0)-f.ms*f.ms;
	    if (arg>0.0 && (f.ms-f.nu-f.m)/temper<-upper_limit_fac) {
	      ll=sqrt(arg);
	    } else {
	      ll=-1.0;
	    }
	  }
      
	  f.ed=dit->integ(mfe,0.0,ul);
	  f.ed*=f.g/2.0/this->pi2;
	  unc.ed=dit->get_error()*f.g/2.0/this->pi2;
      
	  if (ll>0.0) {
	    f.en=dit->integ(mfs,ll,ul);
	    last_method+=4;
	  } else {
	    f.en=dit->integ(mfs,0.0,ul);
	    last_method+=5;
	  }
	  f.en*=f.g/2.0/this->pi2;
	  unc.en=dit->get_error()*f.g/2.0/this->pi2;
      
	} else {

	  f.ed=0.0;
	  f.en=0.0;
	  unc.ed=0.0;
	  unc.en=0.0;
	  O2SCL_ERR2("Zero density in degenerate limit in fermion_rel::",
		     "calc_mu(). Variable deg_limit set improperly?",
		     exc_efailed);
      
	}
      }
      
      f.n=density_temp;
      f.pr=-f.ed+temper*f.en+f.nu*f.n;
      unc.pr=sqrt(unc.ed*unc.ed+temper*unc.en*temper*unc.en+
		  f.nu*unc.n*f.nu*unc.n);
  
      return 0;
    }

    /** \brief Calculate properties with antiparticles as function of
	chemical potential
    */
    void pair_mu(fermion_t &f, fp_t temper) {

      last_method=0;
      
      if (f.non_interacting) { f.nu=f.mu; f.ms=f.m; }
      
      // AWS: 2/12/19: I'm taking this out, similar to the removal
      // of the code in fermion_rel::pair_fun(). If I put it
      // back in, I need to find a new value for last_method
      // other than 9.
      if (false && use_expansions) {
	if (this->calc_mu_ndeg(f,temper,tol_expan,true)) {
	  unc.n=tol_expan*f.n;
	  unc.ed=tol_expan*f.ed;
	  unc.en=tol_expan*f.en;
	  unc.pr=tol_expan*f.pr;
	  last_method=9;
	  return;
	}
      }

      fermion_t antip(f.m,f.g);
      f.anti(antip);

      // Particles
      calc_mu(f,temper);
      fp_t unc_n=unc.n;
      fp_t unc_pr=unc.pr;
      fp_t unc_ed=unc.ed;
      fp_t unc_en=unc.en;

      // Antiparticles
      int lm=last_method*10;
      calc_mu(antip,temper);
      last_method+=lm;
      last_method*=10;

      // Add up thermodynamic quantities
      if (f.inc_rest_mass) {
	f.ed+=antip.ed;
      } else {
	f.ed=f.ed+antip.ed+2.0*antip.n*f.m;
      }
      f.n-=antip.n;
      f.pr+=antip.pr;
      f.en+=antip.en;

      // Add up uncertainties
      unc.n=o2hypot(unc.n,unc_n);
      unc.ed=o2hypot(unc.ed,unc_ed);
      unc.pr=o2hypot(unc.pr,unc_pr);
      unc.en=o2hypot(unc.ed,unc_en);

      return;
    }

    /** \brief Calculate thermodynamic properties with antiparticles
	from the density
    */
    int pair_density(fermion_t &f, fp_t temper) {

      last_method=0;
      
      // -----------------------------------------------------------------
      // Handle T<=0

      if (temper<=0.0) {
	this->calc_density_zerot(f);
	last_method=1000;
	return success;
      }

      // Storage the input density
      fp_t density_temp=f.n;
  
      if (f.non_interacting==true) { f.nu=f.mu; f.ms=f.m; }

      fp_t initial_guess=f.nu;
  
      fp_t nex=f.nu/temper;
      
      func_t mf=std::bind(std::mem_fn<fp_t(fp_t,fermion_t &,fp_t,bool)>
			  (&fermion_rel_tl<fermion_t,fd_inte_t,be_inte_t,
			   nit_t,dit_t,density_root_t,
			   root_t,func_t,fp_t>::pair_fun),
			  this,std::placeholders::_1,std::ref(f),temper,false);

      // Begin by trying the user-specified guess
      bool drec=density_root->err_nonconv;
      density_root->err_nonconv=false;
      int ret=density_root->solve(nex,mf);

      // If that doesn't work, try bracketing the root
      if (ret==0) {
	last_method=2000;
      } else {

	// (std::max doesn't work with boost::multiprecision?)
	fp_t lg;
	if (o2abs(f.nu)>f.ms) lg=o2abs(f.nu);
	lg=f.ms;
	
	fp_t b_high=lg/temper, b_low=-b_high;
	fp_t yhigh=mf(b_high), ylow=mf(b_low);
	for(size_t j=0;j<5 && yhigh<0.0;j++) {
	  b_high*=1.0e2;
	  yhigh=mf(b_high);
	}
	for(size_t j=0;j<5 && ylow>0.0;j++) {
	  b_low*=1.0e2;
	  ylow=mf(b_low);
	}
	if (yhigh>0.0 && ylow<0.0) {
	  root_brent_gsl<func_t,fp_t> rbg;
	  rbg.err_nonconv=false;
	  ret=rbg.solve_bkt(b_low,b_high,mf);
	  if (ret==0) {
	    nex=b_low;
	    last_method=3000;
	  }
	}
      }
  
      if (ret!=0) {

	// If that fails, try to make the integrators more accurate
	fp_t tol1=dit->tol_rel, tol2=dit->tol_abs;
	fp_t tol3=nit->tol_rel, tol4=nit->tol_abs;
	dit->tol_rel/=1.0e2;
	dit->tol_abs/=1.0e2;
	nit->tol_rel/=1.0e2;
	nit->tol_abs/=1.0e2;
	ret=density_root->solve(nex,mf);
	if (ret==0) last_method=4000;
    
	// AWS: 7/25/18: We work in log units below, so we ensure the
	// chemical potential is not negative
	if (nex<0.0) nex=1.0e-10;
    
	// If that failed, try working in log units

	// Function in log units
	func_t lmf=std::bind(std::mem_fn<fp_t(fp_t,fermion_t &,
					      fp_t,bool)>
			     (&fermion_rel_tl<fermion_t,fd_inte_t,be_inte_t,
			      nit_t,dit_t,density_root_t,
			      root_t,func_t,fp_t>::pair_fun),
			     this,std::placeholders::_1,std::ref(f),
			     temper,true);
    
	if (ret!=0) {
	  nex=o2log(nex);
	  ret=density_root->solve(nex,lmf);
	  nex=o2exp(nex);
	  if (ret==0) last_method=5000;
	}
	
	if (ret!=0) {
	  // If that failed, try a different solver
	  root_brent_gsl<func_t,fp_t> rbg;
	  rbg.err_nonconv=false;
	  nex=o2log(nex);
	  ret=rbg.solve(nex,lmf);
	  nex=o2exp(nex);
	  if (ret==0) last_method=6000;
	}

	// Return tolerances to their original values
	dit->tol_rel=tol1;
	dit->tol_abs=tol2;
	nit->tol_rel=tol3;
	nit->tol_abs=tol4;
      }

      // Restore value of err_nonconv
      density_root->err_nonconv=drec;

      if (ret!=0) {
	std::cout.precision(14);
	std::cout << "m,ms,n,T: " << f.m << " " << f.ms << " "
		  << f.n << " " << temper << std::endl;
	std::cout << "nu: " << initial_guess << std::endl;
	O2SCL_CONV2_RET("Density solver failed in fermion_rel::",
			"pair_density().",exc_efailed,this->err_nonconv);
      }

      f.nu=nex*temper;
  
      if (f.non_interacting==true) { f.mu=f.nu; }

      // Finally, now that we have the chemical potential, use pair_mu()
      // to evaluate the energy density, pressure, and entropy
      int lm=last_method;
      pair_mu(f,temper);
      last_method+=lm;

      // The function pair_mu() can modify the density, which would be
      // confusing to the user, so we return it to the user-specified
      // value.
      f.n=density_temp;

      return success;
    }
    //@}

  protected:
    
#ifndef DOXYGEN_INTERNAL

    /// The integrand for the density for non-degenerate fermions
    fp_t density_fun(fp_t u, fermion_t &f, fp_t T) {

      fp_t ret, y, eta;

      if (f.inc_rest_mass) {
	y=f.nu/T;
      } else {
	y=(f.nu+f.m)/T;
      }
      eta=f.ms/T;
  
      fp_t arg1=u*u+2*eta*u;
      fp_t arg2=eta+u-y;
      fp_t arg3=eta+u;
      if (y-eta-u>exp_limit) {
	ret=(eta+u)*o2sqrt(arg1);
      } else if (y>u+exp_limit && eta>u+exp_limit) {
	ret=(eta+u)*o2sqrt(arg1)/(o2exp(arg2)+1.0);
      } else {
	ret=(eta+u)*o2sqrt(arg1)*o2exp(y)/(o2exp(arg3)+o2exp(y));
      }

      if (!o2isfinite(ret)) {
	ret=0.0;
      }

      return ret;
    }

    /// The integrand for the pressure for non-degenerate fermions
    fp_t pressure_fun(fp_t u, fermion_t &f, fp_t T) {

      fp_t ret, y, eta;

      if (f.inc_rest_mass) {
	y=f.nu/T;
      } else {
	y=(f.nu+f.m)/T;
      }
      eta=f.ms/T;
      
      fp_t arg1=u*u+2*eta*u;
      fp_t term1=o2sqrt(arg1);
      fp_t arg3=eta+u;
      ret=term1*term1*term1*o2exp(y)/(o2exp(arg3)+o2exp(y))/3;
      
      if (!o2isfinite(ret)) {
	ret=0.0;
      }

      return ret;
    }

    /// The integrand for the energy density for non-degenerate fermions
    fp_t energy_fun(fp_t u, fermion_t &f, fp_t T) {
      fp_t ret, y, eta;

      eta=f.ms/T;

      if (f.inc_rest_mass) {
	y=f.nu/T;
      } else {
	y=(f.nu+f.m)/T;
      }
      fp_t arg1=u*u+2*eta*u;
      fp_t arg2=eta+u-y;
      fp_t arg3=eta+u;
      if (y>u+exp_limit && eta>u+exp_limit) {
	ret=(eta+u)*(eta+u)*o2sqrt(arg1)/(o2exp(arg2)+1.0);
      } else {
	ret=(eta+u)*(eta+u)*o2sqrt(arg1)*o2exp(y)/
          (o2exp(arg3)+o2exp(y));
      }
 
      if (!o2isfinite(ret)) {
	return 0.0;
      }

      return ret;
    }

    /// The integrand for the entropy density for non-degenerate fermions
    fp_t entropy_fun(fp_t u, fermion_t &f, fp_t T) {
      fp_t ret, y, eta;

      if (f.inc_rest_mass) {
	y=f.nu/T;
      } else {
	y=(f.nu+f.m)/T;
      }
      eta=f.ms/T;

      fp_t arg1=u*u+2*eta*u;
      fp_t arg2=eta+u-y;
      fp_t arg3=eta+u;
      fp_t arg4=y-eta-u;
      fp_t arg5=1+o2exp(arg4);
      fp_t arg6=1+o2exp(arg2);
      fp_t term1=o2log(arg5)/(1+o2exp(arg4));
      fp_t term2=o2log(arg6)/(1+o2exp(arg2));
      ret=(eta+u)*o2sqrt(arg1)*(term1+term2);
  
      if (!o2isfinite(ret)) {
	return 0.0;
      }

      return ret;
    }

    /// The integrand for the density for degenerate fermions
    fp_t deg_density_fun(fp_t k, fermion_t &f, fp_t T) {
      
      fp_t E=o2hypot(k,f.ms), ret;
      if (!f.inc_rest_mass) E-=f.m;

      fp_t arg1=(E-f.nu)/T;
      ret=k*k/(1.0+o2exp(arg1));
      
      if (!o2isfinite(ret)) {
	O2SCL_ERR2("Returned not finite result ",
		   "in fermion_rel::deg_density_fun().",exc_einval);
      }
      
      return ret;
    }

    /// The integrand for the energy density for degenerate fermions
    fp_t deg_energy_fun(fp_t k, fermion_t &f, fp_t T) {

      fp_t E=o2hypot(k,f.ms), ret;
      if (!f.inc_rest_mass) E-=f.m;

      fp_t arg1=(E-f.nu)/T;
      ret=k*k*E/(1.0+o2exp(arg1));

      if (!o2isfinite(ret)) {
	O2SCL_ERR2("Returned not finite result ",
		   "in fermion_rel::deg_energy_fun().",exc_einval);
      }
  
      return ret;
    }

    /// The integrand for the energy density for degenerate fermions
    fp_t deg_pressure_fun(fp_t k, fermion_t &f, fp_t T) {

      fp_t E=o2hypot(k,f.ms), ret;
      if (!f.inc_rest_mass) E-=f.m;

      fp_t arg1=(E-f.nu)/T;
      ret=k*k*k*k/3/E/(1.0+o2exp(arg1));

      if (!o2isfinite(ret)) {
	O2SCL_ERR2("Returned not finite result ",
		   "in fermion_rel::deg_energy_fun().",exc_einval);
      }
  
      return ret;
    }

    /// The integrand for the entropy density for degenerate fermions
    fp_t deg_entropy_fun(fp_t k, fermion_t &f, fp_t T) {
  
      fp_t E=o2hypot(k,f.ms), ret;
      if (!f.inc_rest_mass) E-=f.m;

      // If the argument to the exponential is really small, then the
      // value of the integrand is just zero
      if (((E-f.nu)/T)<-exp_limit) {
	ret=0.0;
	// Otherwise, if the argument to the exponential is still small,
	// then addition of 1 makes us lose precision, so we use an
	// alternative:
      } else if (((E-f.nu)/T)<-deg_entropy_fac) {
	fp_t arg1=E/T-f.nu/T;
	ret=-k*k*(-1.0+arg1)*o2exp(arg1);
      } else {
	fp_t arg1=E/T-f.nu/T;
	fp_t nx=1.0/(1.0+o2exp(arg1));
        fp_t arg2=1.0-nx;
	ret=-k*k*(nx*o2log(nx)+(1.0-nx)*o2log(arg2));
      }

      if (!o2isfinite(ret)) {
	O2SCL_ERR2("Returned not finite result ",
		   "in fermion_rel::deg_entropy_fun().",exc_einval);
      }

      return ret;
    }

    /// Solve for the chemical potential given the density
    fp_t solve_fun(fp_t x, fermion_t &f, fp_t T) {
      fp_t nden, yy;
  
      f.nu=T*x;

      if (f.non_interacting) f.mu=f.nu;

      bool deg=true;
      fp_t psi;
      if (f.inc_rest_mass) {
	psi=(f.nu-f.ms)/T;
      } else {
	psi=(f.nu+(f.m-f.ms))/T;
      }
      if (psi<deg_limit) deg=false;

      // Try the non-degenerate expansion if psi is small enough
      if (use_expansions && psi<min_psi) {
	fp_t ntemp=f.n;
	bool acc=this->calc_mu_ndeg(f,T,tol_expan);
	if (acc) {
	  unc.n=f.n*tol_expan;
	  yy=(ntemp-f.n)/ntemp;
	  f.n=ntemp;
	  return yy;
	}
	f.n=ntemp;
      }

      // Try the degenerate expansion if psi is large enough
      if (use_expansions && psi>20.0) {
	fp_t ntemp=f.n;
	bool acc=this->calc_mu_deg(f,T,tol_expan);
	if (acc) {
	  unc.n=f.n*tol_expan;
	  yy=(ntemp-f.n)/ntemp;
	  f.n=ntemp;
	  return yy;
	}
	f.n=ntemp;
      }

      // Otherwise, directly perform the integration
      if (!deg) {

	func_t mfe=std::bind(std::mem_fn<fp_t(fp_t,fermion_t &,fp_t)>
			     (&fermion_rel_tl<fermion_t,fd_inte_t,
			      be_inte_t,nit_t,dit_t,density_root_t,
			      root_t,func_t,fp_t>::density_fun),
			     this,std::placeholders::_1,std::ref(f),T);
    
	nden=nit->integ_iu(mfe,0.0);
	nden*=f.g*pow(T,3.0)/2.0/this->pi2;
	unc.n=nit->get_error()*f.g*pow(T,3.0)/2.0/this->pi2;
    
	yy=(f.n-nden)/f.n;

      } else {
    
	func_t mfe=std::bind(std::mem_fn<fp_t(fp_t,fermion_t &,fp_t)>
			     (&fermion_rel_tl<fermion_t,fd_inte_t,
			      be_inte_t,nit_t,dit_t,density_root_t,
			      root_t,func_t,fp_t>::deg_density_fun),
			     this,std::placeholders::_1,std::ref(f),T);
    
	fp_t arg;
	if (f.inc_rest_mass) {
	  arg=pow(upper_limit_fac*T+f.nu,2.0)-f.ms*f.ms;
	} else {
	  arg=pow(upper_limit_fac*T+f.nu+f.m,2.0)-f.ms*f.ms;
	}

	fp_t ul;

	if (arg>0.0) {

	  ul=sqrt(arg);
      
	  nden=dit->integ(mfe,0.0,ul);
	  nden*=f.g/2.0/this->pi2;
	  unc.n=dit->get_error()*f.g/2.0/this->pi2;
      
	} else {

	  nden=0.0;

	}

	yy=(f.n-nden)/f.n;
      }
  
      return yy;
    }

    /** \brief Solve for the chemical potential given the density 
	with antiparticles
	
	\future Particles and antiparticles have different degeneracy
	factors, so we separately use the expansions one at a time. It
	is probably better to separately generate a new expansion
	function which automatically handles the sum of particles and
	antiparticles.
    */
    fp_t pair_fun(fp_t x, fermion_t &f, fp_t T, bool log_mode) {

      // Temporary storage for density to match
      fp_t nn_match=f.n;
      // Number density of particles and antiparticles
      fp_t nden_p, nden_ap;

      // -----------------------------------------------------------------

      f.nu=T*x;
      if (log_mode) {
	f.nu=T*o2exp(x);
      }

      // Sometimes the exp() call above causes an overflow, so
      // we avoid extreme values
      if (!o2isfinite(f.nu)) return 3;

      if (f.non_interacting) f.mu=f.nu;

      // -----------------------------------------------------------------
      // First, try the non-degenerate expansion with both particles and
      // antiparticles together

      // AWS: 7/25/18: I'm taking this section out because it doesn't seem
      // to make sense to me, it apparently uses calc_mu_ndeg() which is
      // for particles only, and I'm not sure that's sufficient here. This
      // section also caused problems for the n=0, T!=0 case.

      if (false && use_expansions) {
	if (this->calc_mu_ndeg(f,T,1.0e-8,true) && o2isfinite(f.n)) {
	  fp_t y1=f.n/nn_match-1.0;
	  if (!o2isfinite(y1)) {
	    O2SCL_ERR("Value 'y1' not finite (10) in fermion_rel::pair_fun().",
		      exc_einval);
	  }
	  // Make sure to restore the value of f.n to it's original value,
	  // nn_match
	  f.n=nn_match;
	  return y1;
	}
      }

      // -----------------------------------------------------------------
      // If that doesn't work, evaluate particles and antiparticles 
      // separately. This is the contribution for particles

      bool deg=true;
      fp_t psi;
      if (f.inc_rest_mass) {
	psi=(f.nu-f.ms)/T;
      } else {
	psi=(f.nu+(f.m-f.ms))/T;
      }
      if (psi<deg_limit) deg=false;

      bool particles_done=false;

      // Try the non-degenerate expansion if psi is small enough
      if (use_expansions && psi<min_psi) {
	if (this->calc_mu_ndeg(f,T,1.0e-8) && o2isfinite(f.n)) {
	  particles_done=true;
	  nden_p=f.n;
	  if (!o2isfinite(nden_p)) {
	    O2SCL_ERR2("Value 'nden_p' not finite (1) in ",
		       "fermion_rel::pair_fun().",exc_einval);
	  }
	}
      }
  
      // Try the degenerate expansion if psi is large enough
      if (use_expansions && particles_done==false && psi>20.0) {
	if (this->calc_mu_deg(f,T,1.0e-8) && o2isfinite(f.n)) {
	  particles_done=true;
	  nden_p=f.n;
	  if (!o2isfinite(nden_p)) {
	    O2SCL_ERR2("Value 'nden_p' not finite (2) in",
		       "fermion_rel::pair_fun().",exc_einval);
	  }
	}
      }

      // If neither expansion worked, use direct integration
      if (particles_done==false) {
    
	if (!deg) {
      
	  // Nondegenerate case
      
	  func_t mfe=std::bind(std::mem_fn<fp_t(fp_t,fermion_t &,fp_t)>
			       (&fermion_rel_tl<fermion_t,fd_inte_t,
				be_inte_t,nit_t,dit_t,density_root_t,
				root_t,func_t,fp_t>::density_fun),
			       this,std::placeholders::_1,std::ref(f),T);
      
	  nden_p=nit->integ_iu(mfe,0.0);
	  nden_p*=f.g*pow(T,3.0)/2.0/this->pi2;
	  if (!o2isfinite(nden_p)) {
	    O2SCL_ERR2("Value 'nden_p' not finite (3) in",
		       "fermion_rel::pair_fun().",exc_einval);
	  }
      
	} else {
      
	  // Degenerate case
      
	  func_t mfe=std::bind(std::mem_fn<fp_t(fp_t,fermion_t &,fp_t)>
			       (&fermion_rel_tl<fermion_t,fd_inte_t,
				be_inte_t,nit_t,dit_t,density_root_t,
				root_t,func_t,fp_t>::deg_density_fun),
			       this,std::placeholders::_1,std::ref(f),T);
      
	  fp_t arg;
	  if (f.inc_rest_mass) {
	    arg=pow(upper_limit_fac*T+f.nu,2.0)-f.ms*f.ms;
	  } else {
	    arg=pow(upper_limit_fac*T+f.nu+f.m,2.0)-f.ms*f.ms;
	  }
      
	  fp_t ul;
	  if (arg>0.0) {
	    ul=sqrt(arg);
	    nden_p=dit->integ(mfe,0.0,ul);
	    nden_p*=f.g/2.0/this->pi2;
	  } else {
	    nden_p=0.0;
	  }
      
	  if (!o2isfinite(nden_p)) {
	    O2SCL_ERR2("Value 'nden_p' not finite (4) in",
		       "fermion_rel::pair_fun().",exc_einval);
	  }

	}

	particles_done=true;

	// End of 'if (particles_done==false)'
      }

      // -----------------------------------------------------------------
      // Compute the contribution from the antiparticles

      if (f.inc_rest_mass) {
	f.nu=-T*x;
	if (log_mode) f.nu=-T*o2exp(x);
      } else {
	f.nu=-T*x-2.0*f.m;
	if (log_mode) f.nu=-T*o2exp(x)-2.0*f.m;
      }
      if (f.non_interacting) f.mu=f.nu;

      bool antiparticles_done=false;

      // Evaluate the degeneracy parameter
      deg=true;
      if (f.inc_rest_mass) {
	psi=(f.nu-f.ms)/T;
      } else {
	psi=(f.nu+f.m-f.ms)/T;
      }
      if (psi<deg_limit) deg=false;

      // Try the non-degenerate expansion if psi is small enough
      if (use_expansions && psi<min_psi) {
	if (this->calc_mu_ndeg(f,T,1.0e-8)) {
	  antiparticles_done=true;
	  nden_ap=f.n;
	  if (!o2isfinite(nden_ap)) {
	    O2SCL_ERR2("Value 'nden_ap' not finite (5) in",
		       "fermion_rel::pair_fun().",
		       exc_einval);
	  }
	}
      }

      // Try the degenerate expansion if psi is large enough
      if (use_expansions && antiparticles_done==false && psi>20.0) {
	if (this->calc_mu_deg(f,T,1.0e-8)) {
	  antiparticles_done=true;
	  nden_ap=f.n;
	  if (!o2isfinite(nden_ap)) {
	    O2SCL_ERR2("Value 'nden_ap' not finite (6) in",
		       "fermion_rel::pair_fun().",
		       exc_einval);
	  }
	}
      }

      // If neither expansion worked, use direct integration
      if (antiparticles_done==false) {
    
	if (!deg) {
      
	  // Nondegenerate case
      
	  func_t mf=std::bind(std::mem_fn<fp_t(fp_t,fermion_t &,fp_t)>
			      (&fermion_rel_tl<fermion_t,fd_inte_t,
			       be_inte_t,nit_t,dit_t,density_root_t,
			       root_t,func_t,fp_t>::density_fun),
			      this,std::placeholders::_1,std::ref(f),T);
      
	  nden_ap=nit->integ_iu(mf,0.0);
	  nden_ap*=f.g*pow(T,3.0)/2.0/this->pi2;
	  if (!o2isfinite(nden_ap)) {
	    O2SCL_ERR2("Value 'nden_ap' not finite (7) in",
		       "fermion_rel::pair_fun().",
		       exc_einval);
	  }
      
	} else {
      
	  // Degenerate case
      
	  func_t mf=std::bind(std::mem_fn<fp_t(fp_t,fermion_t &,fp_t)>
			      (&fermion_rel_tl<fermion_t,fd_inte_t,
			       be_inte_t,nit_t,dit_t,density_root_t,
			       root_t,func_t,fp_t>::deg_density_fun),
			      this,std::placeholders::_1,std::ref(f),T);
      
	  fp_t arg;
	  if (f.inc_rest_mass) {
	    arg=pow(upper_limit_fac*T+f.nu,2.0)-f.ms*f.ms;
	  } else {
	    arg=pow(upper_limit_fac*T+f.nu+f.m,2.0)-f.ms*f.ms;
	  }
      
	  fp_t ul;
	  if (arg>0.0) {
	    ul=sqrt(arg);
	    nden_ap=dit->integ(mf,0.0,ul);
	    nden_ap*=f.g/2.0/this->pi2;
	  } else {
	    nden_ap=0.0;
	  }
	  if (!o2isfinite(nden_ap)) {
	    O2SCL_ERR2("Value 'nden_ap' not finite (8) in",
		       "fermion_rel::pair_fun().",
		       exc_einval);
	  }

	}

	antiparticles_done=true;
      }

      fp_t y2;
      // Finish computing the function value
      if (nn_match==0.0) {
	y2=fabs(nden_p-nden_ap)/fabs(nden_p);
      } else {
	y2=(nden_p-nden_ap)/nn_match-1.0;
      }

      if (!o2isfinite(y2)) {
	O2SCL_ERR("Value 'y2' not finite (9) in fermion_rel::pair_fun().",
		  exc_einval);
      }
  
      // Make sure to restore the value of f.n to it's original value,
      // nn_match
      f.n=nn_match;
      return y2;
    }

    
#endif

  };

  /** \brief Double-precision version of 
      \ref o2scl::fermion_rel_tl 
  */
  typedef fermion_rel_tl<> fermion_rel;

#ifdef O2SCL_LD_TYPES

  /** \brief Equation of state for a relativistic fermion using long 
      double precision
   */
  class fermion_rel_ld : public
  fermion_rel_tl<
    fermion_tl<long double>,
    fermi_dirac_integ_direct<
      long double,funct_cdf35,25,
      boost::multiprecision::number<
	boost::multiprecision::cpp_dec_float<35> > >,
    bessel_K_exp_integ_direct<
      long double,funct_cdf35,25,
      boost::multiprecision::number<
	boost::multiprecision::cpp_dec_float<35> > >,
    inte_tanh_sinh_boost<funct_ld,25,long double>,
    inte_tanh_sinh_boost<funct_ld,25,long double>,
    root_brent_gsl<funct_ld,long double>,
    root_brent_gsl<funct_ld,long double>,
    funct_ld,long double> {
    
  public:
    
    fermion_rel_ld() {

      // See output of polylog_ts for numeric limit information
      
      // Tolerance for the integrator for massless fermions
      this->fd_integ.set_tol(1.0e-21);

      // Tolerance for the integrator for the nondegenerate expansion
      this->be_integ.set_tol(1.0e-21);

      // Internal function tolerances

      // This could be as large as log(1.0e4932)=11400,
      // but only 200 is used for double, so we try this for now.
      this->exp_limit=4000.0;
      
      // log(1.0e18) is 41.4
      this->upper_limit_fac=42.0;
      this->deg_entropy_fac=42.0;
      this->tol_expan=1.0e-17;

      // Solver tolerances
      this->def_density_root.tol_abs=1.0e-18;
      this->def_massless_root.tol_abs=1.0e-18;

      // Integrator tolerances
      this->def_dit.tol_abs=1.0e-16;
      this->def_dit.tol_rel=1.0e-16;
      this->def_nit.tol_abs=1.0e-15;
      this->def_nit.tol_rel=1.0e-15;
     
    }
    
  };
  
  /** \brief Equation of state for a relativistic fermion using 
      35 decimal digit precision
   */
  class fermion_rel_cdf35 : public
  fermion_rel_tl<
    fermion_tl<
      boost::multiprecision::number<
	boost::multiprecision::cpp_dec_float<35> > >,
    fermi_dirac_integ_direct<
      boost::multiprecision::number<
	boost::multiprecision::cpp_dec_float<35> >,
      funct_cdf50,30,
      boost::multiprecision::number<
	boost::multiprecision::cpp_dec_float<50> > >,
    bessel_K_exp_integ_direct
    <boost::multiprecision::number<
       boost::multiprecision::cpp_dec_float<35> >,
     funct_cdf50,30,
     boost::multiprecision::number<
       boost::multiprecision::cpp_dec_float<50> > >,
    inte_tanh_sinh_boost<
      funct_cdf35,30,
      boost::multiprecision::number<
	boost::multiprecision::cpp_dec_float<35> > >,
    inte_tanh_sinh_boost<
      funct_cdf35,30,
      boost::multiprecision::number<
	boost::multiprecision::cpp_dec_float<35> > >,
    root_brent_gsl<
      funct_cdf35,
      boost::multiprecision::number<
	boost::multiprecision::cpp_dec_float<35> > >,
    root_brent_gsl<
      funct_cdf35,boost::multiprecision::number<
		    boost::multiprecision::cpp_dec_float<35> > >,
    funct_cdf35,boost::multiprecision::number<
		  boost::multiprecision::cpp_dec_float<35> > > {
    
  public:
    
    fermion_rel_cdf35() {
      
      // See output of polylog_ts for numeric limit information

      // Tolerance for the integrator for massless fermions
      this->fd_integ.set_tol(1.0e-60);

      // Tolerance for the integrator for the nondegenerate expansion
      this->be_integ.set_tol(1.0e-60);

      // Internal function tolerances
      this->exp_limit=6.0e7;
      this->upper_limit_fac=81.0;
      this->deg_entropy_fac=81.0;
      this->tol_expan=1.0e-35;

      // Solver tolerances
      this->def_density_root.tol_abs=1.0e-35;
      this->def_massless_root.tol_abs=1.0e-35;

      // Integrator tolerances
      this->def_dit.tol_abs=1.0e-35;
      this->def_dit.tol_rel=1.0e-35;
      this->def_nit.tol_abs=1.0e-34;
      this->def_nit.tol_rel=1.0e-34;
    }
  };
  
#endif
  
#ifndef DOXYGEN_NO_O2NS
}
#endif

#endif
