/*
  -------------------------------------------------------------------
  
  Copyright (C) 2020, Andrew W. Steiner
  
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
#ifndef O2SCL_AUTO_FORMAT_H
#define O2SCL_AUTO_FORMAT_H
/** \file auto_format.h
    \brief Desc
*/

#include <iostream>
#include <string>
#include <vector>

#include <o2scl/err_hnd.h>

#ifndef DOXYGEN_NO_O2NS
namespace o2scl_auto_format {
#endif

  /** \brief Automatically format output 

      \future Allow output to files ("fout") in addition to cout
   */
  class auto_format {
    
  protected:

    /// Output line buffer
    std::vector<std::string> lines;

    /// Maximum number of table rows (default 1000)
    size_t row_max;

    /// Alignment specifications for table columns
    std::vector<int> aligns;

    /// If true, we are currently inside a table
    bool inside_table;

    /// Columns for table mode
    std::vector<std::vector<std::string> > columns;

    /// Index of next column
    size_t next_column;

    /// The number of table header rows
    size_t n_headers;
    
    /// Headers for table mode
    std::vector<std::vector<std::string> > headers;

    /// If true, try to automatically detect tables (default true)
    bool auto_tables;
    
  public:

    auto_format();
    
    /** \brief Add a string to the output buffer
     */
    void add_string(std::string s, bool endl=false);

    /** \brief Flush all buffered output to the screen
     */
    void done();

    /// Verbosity parameter (default 0)
    int verbose;

  };

  /** \brief Output a double-precision number
   */
  auto_format &operator<<(auto_format &c, double d);

  /** \brief Output a single-precision number
   */
  auto_format &operator<<(auto_format &c, float f);

  /** \brief Output an integer
   */
  auto_format &operator<<(auto_format &c, int i);

  /** \brief Output a character
   */
  auto_format &operator<<(auto_format &c, char ch);

  /** \brief Output a \c size_t
   */
  auto_format &operator<<(auto_format &c, size_t s);

  /** \brief Output a string
   */
  auto_format &operator<<(auto_format &c, std::string s);

  /** \brief Desc
   */
  //auto_format &operator<<(auto_format &c, o2scl_auto_format::auto_format &(*f)(o2scl_auto_format::auto_format&));

  /** \brief Desc
   */
  //auto_format &endl(auto_format &c);

  /// End the current line
  static const char endo='\n';
  
#ifndef DOXYGEN_NO_O2NS
}
#endif

#endif
