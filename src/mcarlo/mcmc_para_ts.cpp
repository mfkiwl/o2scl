/*
  -------------------------------------------------------------------
  
  Copyright (C) 2016-2018, Andrew W. Steiner
  
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
#include <o2scl/mcmc_para.h>
#include <o2scl/vec_stats.h>
#include <o2scl/test_mgr.h>
#include <o2scl/mcarlo_miser.h>
#include <o2scl/multi_funct.h>
#include <o2scl/expval.h>
#include <o2scl/hdf_io.h>

using namespace std;
using namespace o2scl;
using namespace o2scl_hdf;

#ifdef O2SCL_OLDER_COMPILER

int main(void) {
  test_mgr t;
  t.report();
  return 0;
}

#else

typedef boost::numeric::ublas::vector<double> ubvector;
typedef boost::numeric::ublas::matrix<double> ubmatrix;

typedef std::function<int(size_t,const ubvector &,double &,
			  std::array<double,1> &)> point_funct;

typedef std::function<int(const ubvector &,double,size_t,int,bool,
			  std::array<double,1> &)> measure_funct;

typedef std::function<int(const ubvector &,double,std::vector<double> &,
			  std::array<double,1> &)> fill_funct;

class mcmc_para_class {

public:

  int count;
  
  mcmc_para_base<point_funct,measure_funct,std::array<double,1>,ubvector> mc;

  mcmc_para_table<point_funct,fill_funct,std::array<double,1>,ubvector> mct;

  expval_scalar sev_x, sev_x2;

  int gauss(size_t nv, const ubvector &pars, double &ret,
	    std::array<double,1> &dat) {
    dat[0]=pars[0]*pars[0];
    ret=-pars[0]*pars[0]/2.0;
    return o2scl::success;
  }

  int flat(size_t nv, const ubvector &pars, double &ret,
	    std::array<double,1> &dat) {
    dat[0]=pars[0]*pars[0];
    ret=0.0;
    if (count>0 && count%10000==0) {
      cout << "Count: " << count << endl;
    }
    count++;
    return o2scl::success;
  }

  double f0(size_t nv, const ubvector &pars) {
    return exp(-pars[0]*pars[0]/2.0);
  }

  double f1(size_t nv, const ubvector &pars) {
    return exp(-pars[0]*pars[0]/2.0)*pars[0]*pars[0];
  }

  double f3(size_t nv, const ubvector &pars) {
    return 1.0;
  }

  int measure(const ubvector &pars, double log_weight, size_t ix,
	      int ret, bool new_meas, std::array<double,1> &dat) {
    /*
      sev_x.add(pars[0]);
      sev_x2.add(dat[0]);
      // Check that the 'dat' object is correctly filled
      if ((pars[0]*pars[0]-dat[0])>1.0e-10) {
      cerr << "Failure." << endl;
      exit(-1);
      }
    */
    return 0;
  }

  int fill_func(const ubvector &pars, double log_weight,
		std::vector<double> &line, std::array<double,1> &dat) {
    line.push_back(dat[0]*dat[0]);
    return 0;
  }
  
};

int main(int argc, char *argv[]) {
  
  cout.setf(ios::scientific);

  test_mgr tm;
  tm.set_output_level(1);

  mcmc_para_class mpc;
  
  // Domain limits
  ubvector low(1);
  ubvector high(1);
  low[0]=-5.0;
  high[0]=2.0;

  // Compute exact results
  mcarlo_miser<> mm;
  mm.n_points=100000;
  double res[2], err[2];
  
  multi_funct mf0=std::bind
    (std::mem_fn<double(size_t,const ubvector &)>(&mcmc_para_class::f0),
     &mpc,std::placeholders::_1,std::placeholders::_2);
  multi_funct mf1=std::bind
    (std::mem_fn<double(size_t,const ubvector &)>(&mcmc_para_class::f1),
     &mpc,std::placeholders::_1,std::placeholders::_2);
  
  mm.minteg_err(mf0,1,low,high,res[0],err[0]);
  mm.minteg_err(mf1,1,low,high,res[1],err[1]);
  res[1]/=res[0];
  cout << "Exact results:" << endl;
  cout << res[1] << endl;
  cout << endl;
  
  // Set up MCMC
  point_funct gauss_func=std::bind
    (std::mem_fn<int(size_t,const ubvector &,double &,
		     std::array<double,1> &)>(&mcmc_para_class::gauss),
     &mpc,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3,
     std::placeholders::_4);
  point_funct flat_func=std::bind
    (std::mem_fn<int(size_t,const ubvector &,double &,
		     std::array<double,1> &)>(&mcmc_para_class::flat),
     &mpc,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3,
     std::placeholders::_4);
  fill_funct ff=std::bind
    (std::mem_fn<int(const ubvector &,double,std::vector<double> &,
		     std::array<double,1> &)>(&mcmc_para_class::fill_func),
     &mpc,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3,
     std::placeholders::_4);
  measure_funct mf=std::bind
    (std::mem_fn<int(const ubvector &,double,size_t,int,bool,
		     std::array<double,1> &)>(&mcmc_para_class::measure),&mpc,
     std::placeholders::_1,std::placeholders::_2,std::placeholders::_3,
     std::placeholders::_4,std::placeholders::_5,std::placeholders::_6);

  size_t n_threads=1;
#ifdef O2SCL_OPENMP
  n_threads=2;
#endif

  vector<point_funct> gauss_vec(n_threads);
  vector<point_funct> flat_vec(n_threads);
  vector<measure_funct> vmf(n_threads);
  vector<fill_funct> vff(n_threads);
  for(size_t i=0;i<n_threads;i++) {
    gauss_vec[i]=gauss_func;
    flat_vec[i]=flat_func;
    vmf[i]=mf;
    vff[i]=ff;
  }
  
  ubvector init(1);
  init[0]=-0.01;

  cout << "n_threads: " << n_threads << endl;
  cout << endl;

  // ----------------------------------------------------------------
  // Plain MCMC

  cout << "Plain MCMC: " << endl;
  
  mpc.mc.step_fac=-1.0;
  mpc.mc.verbose=2;
  mpc.mc.n_threads=n_threads;
  mpc.mc.max_iters=40;
  mpc.mc.mcmc(1,low,high,gauss_vec,vmf);
  
  tm.test_gen(mpc.mc.n_accept[0]+mpc.mc.n_reject[0]==mpc.mc.max_iters,
	      "plain n_iters 0");
  if (n_threads>1) {
    tm.test_gen(mpc.mc.n_accept[1]+mpc.mc.n_reject[1]==mpc.mc.max_iters,
	      "plain n_iters 1");
  }
  cout << endl;

  // ----------------------------------------------------------------
  // Affine-invariant MCMC

  cout << "Affine-invariant MCMC: " << endl;
  
  mpc.mc.aff_inv=true;
  mpc.mc.n_walk=10;
  mpc.mc.step_fac=-1.0;
  mpc.mc.verbose=2;
  mpc.mc.n_threads=n_threads;
  
  mpc.mc.max_iters=40;
  mpc.mc.prefix="mcmc_ai";
  mpc.mc.mcmc(1,low,high,gauss_vec,vmf);

  tm.test_gen(mpc.mc.n_accept[0]+mpc.mc.n_reject[0]==mpc.mc.max_iters,
	      "aff_inv n_iters 0");
  if (n_threads>1) {
    tm.test_gen(mpc.mc.n_accept[1]+mpc.mc.n_reject[1]==mpc.mc.max_iters,
		"aff_inc n_iters 1");
  }
  cout << endl;

  // ----------------------------------------------------------------
  // Plain MCMC with a table

  cout << "Plain MCMC with a table: " << endl;
  
  vector<string> pnames={"x","x2"};
  vector<string> punits={"MeV","MeV^2"};
  mpc.mct.set_names_units(pnames,punits);

  mpc.mct.step_fac=-1.0;
  mpc.mct.verbose=2;
  mpc.mct.n_threads=n_threads;
  mpc.mct.max_iters=40;
  mpc.mct.prefix="mcmct";
  mpc.mct.mcmc(1,low,high,gauss_vec,vff);

  std::vector<size_t> chain_sizes;
  std::shared_ptr<o2scl::table_units<> > table=mpc.mct.get_table();

  mpc.mct.get_chain_sizes(chain_sizes);
  tm.test_gen(chain_sizes[0]==mpc.mct.n_accept[0]+1,"plain table size 0");
  if (n_threads>1) {
    tm.test_gen(chain_sizes[1]==mpc.mct.n_accept[1]+1,"plain table size 1");
  }
  tm.test_gen(mpc.mct.n_accept[0]+mpc.mct.n_reject[0]==mpc.mct.max_iters,
	      "plain table n_iters 0");
  if (n_threads>1) {
    tm.test_gen(mpc.mct.n_accept[1]+mpc.mct.n_reject[1]==mpc.mct.max_iters,
		"plain table n_iters 1");
  }
    
  // ----------------------------------------------------------------
  // Affine-invariant MCMC with a table
  
  cout << "Affine-invariant MCMC with a table: " << endl;
  
  mpc.mct.aff_inv=true;
  mpc.mct.n_walk=10;
  mpc.mct.step_fac=-1.0;
  
  mpc.mct.max_iters=40;
  mpc.mct.prefix="mcmct_ai";
  mpc.mct.mcmc(1,low,high,gauss_vec,vff);

  // Get results
  table=mpc.mct.get_table();
  mpc.mct.get_chain_sizes(chain_sizes);

  // Test results
  tm.test_gen(vector_sum_double(mpc.mct.n_walk,chain_sizes)-mpc.mct.n_walk==
	      mpc.mct.n_accept[0],"accept chain 0");
  if (n_threads>1) {
    tm.test_gen(vector_sum_double(mpc.mct.n_walk*2,chain_sizes)-
		vector_sum_double(mpc.mct.n_walk,chain_sizes)-mpc.mct.n_walk==
		mpc.mct.n_accept[1],"accept chain 1");
  }
  tm.test_gen(mpc.mct.n_accept[0]+mpc.mct.n_reject[0]==mpc.mct.max_iters,
	      "aff_inv table n_iters 0");
  if (n_threads>1) {
    tm.test_gen(mpc.mct.n_accept[1]+mpc.mct.n_reject[1]==mpc.mct.max_iters,
		"aff_inv table n_iters 1");
  }

  // ----------------------------------------------------------------
  // Plain MCMC with a table and a flat distribution
  
  cout << "Plain MCMC with a table and a flat distribution: "
       << endl;

  mpc.count=0;
  mpc.mct.aff_inv=false;
  mpc.mct.n_walk=1;
  mpc.mct.step_fac=10.0;
  mpc.mct.verbose=2;
  
  mpc.mct.max_iters=10000;
  mpc.mct.prefix="mcmct_flat";
  mpc.mct.mcmc(1,low,high,flat_vec,vff);

  // ----------------------------------------------------------------
  // Affine-invariant MCMC with a table and a flat distribution
  
  cout << "Affine-invariant MCMC with a table and a flat distribution: "
       << endl;
  
  mpc.mct.aff_inv=true;
  mpc.mct.n_walk=10;
  mpc.mct.step_fac=-1.0;
  mpc.mct.verbose=2;
  
  mpc.mct.max_iters=10000;
  mpc.mct.prefix="mcmct_ai_flat";
  mpc.mct.mcmc(1,low,high,flat_vec,vff);

  // ----------------------------------------------------------------
  // Affine-invariant MCMC with a table and previously read results
  
  cout << "Affine-invariant MCMC with a table and previous results: "
       << endl;
  
  mpc.mct.aff_inv=true;
  mpc.mct.n_walk=10;
  mpc.mct.step_fac=-1.0;
  
  mpc.mct.max_iters=40;
  mpc.mct.prefix="mcmct_aiprev";

  hdf_file hf;
  string fname="mcmct_ai_0_out";
  hf.open(fname);
  mpc.mct.read_prev_results(hf,1);
  hf.close();

  cout << "Going to mcmc." << endl;
  mpc.mct.verbose=1;
  mpc.mct.mcmc(1,low,high,gauss_vec,vff);

  // Get results
  table=mpc.mct.get_table();
  std::vector<size_t> chain_sizes2;
  mpc.mct.get_chain_sizes(chain_sizes2);

  // This testing code doesn't work yet, possibly because it is
  // not yet written correctly
  for(size_t it=0;it<n_threads;it++) {
    size_t sum1=0, sum2=0;
    for(size_t i=0;i<10;i++) {
      sum1+=chain_sizes[it*10+i];
    }
    for(size_t i=0;i<10;i++) {
      sum2+=chain_sizes2[it*10+i];
    }
    cout << sum1 << " " << sum2 << " " << sum2-sum1 << " " 
	 << mpc.mct.n_accept[it] << " "
	 << mpc.mct.n_reject[it] << endl;
    //tm.test_gen(sum2-sum1==mpc.mct.n_accept[it],"Test chain size");
  }
  cout << endl;
  
  tm.report();
  
  return 0;
}

#endif
