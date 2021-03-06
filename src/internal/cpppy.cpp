/*
  -------------------------------------------------------------------
  
  Copyright (C) 2020-2021, Andrew W. Steiner
  
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
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <o2scl/string_conv.h>

using namespace std;
using namespace o2scl;

/*
  Get and set methods for C++ class members always imply a copy. Right
  now cpppy does not yet support, for example, obtaining a reference
  or a pointer to a c++ class member.

  Todos: need to fix function names in case where there is no namespace.
*/

/** \brief Convert all non-alphanumeric characters to underscores
 */
std::string underscoreify(std::string s) {
  std::string s2=s;
  for(size_t i=0;i<s2.length();i++) {
    if (std::isalnum(s2[i])==false) s2[i]='_';
  }
  return s2;
}

/** \brief Interface base
 */
class if_base {
  
public:
  
  std::string name;
  
};

/** \brief Interface type
 */
class if_type : public if_base {
  
public:
  
  // Const, static, etc.
  std::string prefix;
  
  // *, *&, &, etc.
  std::string suffix;

  /** \brief Desc
   */
  std::string to_string() {
    std::string s=this->name;
    if (prefix.length()>0) s=prefix+" "+s;
    if (suffix.length()>0) s+=" "+suffix;
    return s;
  }
  
  /** \brief Parse a vector string object into the type parts
   */
  void parse(std::vector<std::string> &vs, size_t start, size_t end) {
    
    // Create a new vector string object for convenience
    std::vector<std::string> vs2;
    for(size_t i=start;i<end;i++) {
      vs2.push_back(vs[i]);
    }
    
    // Handle according to the number of arguments
    if (vs2.size()==1) {
      prefix="";
      suffix="";
      name=vs2[0];
    } else if (vs2.size()==2) {
      bool symb_only=true;
      for(size_t i=0;i<vs2[1].length();i++) {
        if (vs2[1][i]!='*' && vs2[1][i]!='&') symb_only=false;
      }
      if (symb_only) {
        name=vs2[0];
        suffix=vs2[1];
      } else {
        prefix=vs2[0];
        name=vs2[1];
      }
    } else if (vs2.size()==3) {
      bool symb_only=true;
      for(size_t i=0;i<vs2[2].length();i++) {
        if (vs2[2][i]!='*' && vs2[2][i]!='&') symb_only=false;
      }
      if (symb_only) {
        prefix=vs2[0];
        name=vs2[1];
        suffix=vs2[2];
      } else {
        prefix=vs2[0]+" "+vs2[1];
        name=vs2[2];
      }
    } else {
      cerr << "Unsupported number of type arguments." << endl;
      exit(-1);
    }
    
    // Take *'s and &'s in name and add them to suffix
    char last_ch=name[name.length()-1];
    while (last_ch=='*' && last_ch=='&') {
      string stemp;
      stemp+=last_ch;
      suffix=stemp+suffix;
      if (name.length()==1) {
        last_ch=' ';
      } else {
        name.substr(0,name.length()-1);
        last_ch=name[name.length()-1];
      }
    }
    
    return;
  }

  /// Return true if the type is a pointer
  bool is_pointer() {
    if (suffix=="*") return true;
  }
  
  /// Return true if the type is a reference
  bool is_reference() {
    if (suffix=="&") return true;
  }
  
  /// Return true if the type is const
  bool is_const() {
    if (prefix.find("const")!=std::string::npos) return true;
  }
  
  /// Return true if the type is static
  bool is_static() {
    if (prefix.find("const")!=std::string::npos) return true;
  }

  // End of class if_type
};

/** \brief A variable with a type and a name
 */
class if_var : public if_base {
  
public:

  std::string py_name;

  /// The variable type 
  if_type ift;
  
};

/** \brief Interface function
 */
class if_func : public if_base {
  
public:
  
  /// Return value 
  if_type ret;

  /// Function arguments
  std::vector<if_var> args;
  
  /// Namespace
  std::string ns;
  
};

/** \brief Interface class
 */
class if_class : public if_base {
  
public:

  /// Python name for the class
  std::string py_name;

  /// Pattern for the class documentation in python
  std::string py_class_doc_pattern;
  
  /// True if the class is abstract
  bool is_abstract;
  
  /// Members
  std::vector<if_var> members;
  
  /// Methods
  std::vector<if_func> methods;
  
  /// Constructors
  std::vector<if_func> cons;
  
  /// List of parent classes
  std::vector<std::string> parents;
  
  /// Namespace
  std::string ns;

  if_class() {
    is_abstract=false;
  }
  
};

/** \brief Read the next line from \c fin, skipping blank
    lines, splitting by spaces into \c vs, and setting \c done
    if done
*/
void next_line(ifstream &fin, std::string &line,
               std::vector<std::string> &vs, bool &done) {
  if (fin.eof()) {
    done=true;
    return;
  }
  getline(fin,line);
  while (line.length()==0 || line[0]=='#') {
    if (fin.eof()) {
      done=true;
      return;
    } else {
      getline(fin,line);
    }
  }
  vs.clear();
  split_string(line,vs);
  return;
}

int main(int argc, char *argv[]) {

  if (argc<4) {
    cerr << "Args: <interface file> <c++ output prefix> "
         << "<python prefix>" << endl;
    exit(-1);
  }
  std::string fname=argv[1];
  std::string cpp_prefix=argv[2];
  std::string py_prefix=argv[3];

  cout << "Reading interface file " << fname << " ." << endl;
  cout << "Setting C++ output prefix to " << cpp_prefix << " ." << endl;
  cout << "Setting Python output prefix to " << py_prefix << " ." << endl;
  cout << endl;

  vector<string> header=
    {"  -------------------------------------------------------------------",
     "",
     "  Copyright (C) 2020-2021, Andrew W. Steiner",
     "",
     "  This file is part of O2scl.",
     "",
     "  O2scl is free software; you can redistribute it and/or modify",
     "  it under the terms of the GNU General Public License as published by",
     "  the Free Software Foundation; either version 3 of the License, or",
     "  (at your option) any later version.",
     "",
     "  O2scl is distributed in the hope that it will be useful,",
     "  but WITHOUT ANY WARRANTY; without even the implied warranty of",
     "  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the",
     "  GNU General Public License for more details.",
     "",
     "  You should have received a copy of the GNU General Public License",
     "  along with O2scl. If not, see <http://www.gnu.org/licenses/>."
     "",
     "  -------------------------------------------------------------------"
    };
  
  cout << "Parsing interface " << fname << " ." << endl;
  
  // Current namespace
  std::string ns;
  std::string py_class_doc_pattern;
  // Current list of includes
  std::vector<std::string> h_includes;
  // Current list of includes
  std::vector<std::string> cpp_includes;
  // Current list of includes
  std::vector<std::string> py_headers;
  // Current list of includes
  std::vector<std::string> cpp_using;
  // Current list of classes
  std::vector<if_class> classes;
  // Current list of functions
  std::vector<if_func> functions;
  
  // Open file
  ifstream fin;
  fin.open(fname.c_str());
  
  // Parse file
  std::string line;
  std::vector<std::string> vs;
  bool done=false;
  
  next_line(fin,line,vs,done);
  if (done==true) {
    cerr << "Empty interface file." << endl;
    exit(-1);
  }
  
  while (done==false) {

    if (vs[0]=="namespace") {
      if (vs.size()==1) {
        ns="";
        cout << "Clearing namespace." << endl;
      } else {
        ns=vs[1];
        cout << "Setting namespace to " << ns << "." << endl;
      }
      
      next_line(fin,line,vs,done);
      
    } else if (vs[0]=="py_class_doc_pattern") {
      py_class_doc_pattern="";
      if (vs.size()==1) {
        cout << "Clearing py_class_doc_pattern." << endl;
      } else {
        for(size_t k=1;k<vs.size();k++) {
          py_class_doc_pattern+=vs[k];
          if (k!=vs.size()-1) py_class_doc_pattern+=" ";
        }
        if (py_class_doc_pattern[0]=='\"' &&
            py_class_doc_pattern[py_class_doc_pattern.length()-
                                 1]=='\"') {
          py_class_doc_pattern=py_class_doc_pattern.substr
            (1,py_class_doc_pattern.length()-2);
        }
        cout << "Setting py_class_doc_pattern to \""
             << py_class_doc_pattern << "\"." << endl;
      }
      
      next_line(fin,line,vs,done);
      
    } else if (vs[0]=="h_include") {
      
      if (vs.size()<2) {
        cerr << "No argument for h_include." << endl;
        exit(-1);
      }
      h_includes.push_back(vs[1]);
      
      next_line(fin,line,vs,done);
      
    } else if (vs[0]=="py_header") {
      
      if (vs.size()<2) {
        cerr << "No argument for py_header." << endl;
        exit(-1);
      }
      std::string pyh=vs[1];
      for(size_t j=2;j<vs.size();j++) {
        pyh+=" "+vs[j];
      }
      py_headers.push_back(pyh);
      
      next_line(fin,line,vs,done);
      
    } else if (vs[0]=="cpp_include") {
      
      if (vs.size()<2) {
        cerr << "No argument for cpp_include." << endl;
        exit(-1);
      }
      cpp_includes.push_back(vs[1]);
      
      next_line(fin,line,vs,done);
      
    } else if (vs[0]=="cpp_using") {
      
      if (vs.size()<2) {
        cerr << "No argument for cpp_using." << endl;
        exit(-1);
      }
      cpp_using.push_back(vs[1]);
      
      next_line(fin,line,vs,done);
      
    } else if (vs[0]=="class") {
      
      if (vs.size()<2) {
        cerr << "No name for class." << endl;
        exit(-1);
      }
      if_class ifc;
      ifc.name=vs[1];
      ifc.ns=ns;
      if (vs.size()>=3 && vs[2]=="abstract") {
        ifc.is_abstract=true;
        cout << "Starting abstract class " << ifc.name << " in namespace "
             << ifc.ns << endl;
      } else {
        cout << "Starting class " << ifc.name << " in namespace "
             << ifc.ns << endl;
      }
      
      next_line(fin,line,vs,done);
      
      bool class_done=false;
      
      while (class_done==false) {

        if (vs.size()>=3 && vs[0]=="-" && vs[1]=="function") {
          
          if_func iff;
          iff.name=vs[2];
          cout << "  Starting member function " << vs[2] << endl;

          next_line(fin,line,vs,done);
          
          if (done==true || vs.size()<2 || vs[0]!="-" || line[0]!=' ' ||
              line[1]!=' ') {
            cerr << "Could not get return value for function "
                 << iff.name << " in class " << ifc.name << endl;
            exit(-1);
          }
          
          iff.ret.parse(vs,1,vs.size());

          cout << "    Member function " << iff.name
               << " has return type "
               << iff.ret.to_string() << endl;

          next_line(fin,line,vs,done);
          
          while (vs.size()>=2 && line[0]==' ' && line[1]==' ' &&
                 vs[0]=="-") {
            
            if_var ifv;
            string last_string=vs[vs.size()-1];
            if (last_string[0]=='&' || last_string[0]=='*') {
              vs[vs.size()-1]="";
              while (last_string[0]=='&' || last_string[0]=='*') {
                vs[vs.size()-1]=last_string[0]+vs[vs.size()-1];
                last_string=last_string.substr(1,last_string.length()-1);
              }
              ifv.name=last_string;
              ifv.ift.parse(vs,1,vs.size());
            } else {
              ifv.name=last_string;
              ifv.ift.parse(vs,1,vs.size()-1);
            }
            cout << "    Member function " << iff.name
                 << " has argument " << ifv.name << " with type "
                 << ifv.ift.to_string() << endl;

            iff.args.push_back(ifv);
            
            next_line(fin,line,vs,done);
            if (done) class_done=true;
          }

          ifc.methods.push_back(iff);
          
        } else if (vs.size()>=3 && vs[0]=="-" && vs[1]=="parent") {
          
          ifc.parents.push_back(vs[2]);
          cout << "  Parent class " << vs[2] << endl;
          
          next_line(fin,line,vs,done);
          if (done) class_done=true;
          
        } else if (vs.size()>=3 && vs[0]=="-" &&
                   vs[1]=="py_class_doc_pattern") {
          
          ifc.py_class_doc_pattern=vs[2];
          for(size_t k=3;k<vs.size();k++) {
            ifc.py_class_doc_pattern+=" "+vs[k];
          }
          if (ifc.py_class_doc_pattern[0]=='\"' &&
              ifc.py_class_doc_pattern[ifc.py_class_doc_pattern.length()-
                                       1]=='\"') {
            ifc.py_class_doc_pattern=ifc.py_class_doc_pattern.substr
              (1,ifc.py_class_doc_pattern.length()-2);
          }
          cout << "  Class py_class_doc_pattern is \""
               << ifc.py_class_doc_pattern << "\"." << endl;
          
          next_line(fin,line,vs,done);
          if (done) class_done=true;
          
        } else if (vs.size()>=3 && vs[0]=="-") {
          
          if_var ifv;
          // First, check if there are any & or *'s before the
          // variable name
          if (vs[2][0]=='&' || vs[2][0]=='*') {
            ifv.name=vs[2].substr(1,vs[2].length()-1);
            vs[2]=vs[2].substr(0,1);
            ifv.ift.parse(vs,1,3);
          } else {
            ifv.name=vs[2];
            ifv.ift.parse(vs,1,2);
          }
          
          cout << "  Member " << ifv.name << " with type "
               << ifv.ift.to_string() << " ." << endl;

          next_line(fin,line,vs,done);
          if (done) {
            class_done=true;
          } else {
            if (line[0]==' ' && line[1]==' ' && line[2]=='-' &&
                line[3]==' ' && vs[1]=="py_name" && vs.size()>=3) {
              ifv.py_name=vs[2];
              cout << "    Member " << ifv.name << " has py_name "
                   << ifv.py_name << " ." << endl;
              next_line(fin,line,vs,done);
              if (done) {
                class_done=true;
              }
            }
          }
          
          ifc.members.push_back(ifv);

        } else {
          class_done=true;
        }

        if (class_done==true) {
          cout << "Class " << ifc.name << " done." << endl;
          cout << endl;
          classes.push_back(ifc);
        }

        // End of 'while (class_done==false)'
      }
      
    } else if (vs[0]=="function") {

      if_func iff;
      iff.name=vs[1];
      iff.ns=ns;
      cout << "Starting function " << vs[1] << endl;
      
      next_line(fin,line,vs,done);
          
      if (done==true || vs.size()<2 || vs[0]!="-" || line[0]!='-' ||
          line[1]!=' ') {
        cerr << "Could not get return value for function "
             << iff.name << endl;
        exit(-1);
      }
          
      iff.ret.parse(vs,1,vs.size());
      
      cout << "  Member function " << iff.name
           << " has return type "
           << iff.ret.to_string() << endl;
      
      next_line(fin,line,vs,done);
      
      bool function_done=false;
      
      while (vs.size()>=2 && line[0]=='-' && line[1]==' ' &&
             vs[0]=="-") {
        
        if_var ifv;
        string last_string=vs[vs.size()-1];
        if (last_string[0]=='&' || last_string[0]=='*') {
          vs[vs.size()-1]="";
          while (last_string[0]=='&' || last_string[0]=='*') {
            vs[vs.size()-1]=last_string[0]+vs[vs.size()-1];
            last_string=last_string.substr(1,last_string.length()-1);
          }
          ifv.name=last_string;
          ifv.ift.parse(vs,1,vs.size());
        } else {
          ifv.name=last_string;
          ifv.ift.parse(vs,1,vs.size()-1);
        }
        cout << "  Function " << iff.name
             << " has argument " << ifv.name << " with type "
             << ifv.ift.to_string() << endl;
        
        iff.args.push_back(ifv);
        
        next_line(fin,line,vs,done);
        if (done) function_done=true;
      }
      
      cout << "Function " << iff.name << " done." << endl;
      cout << endl;
      functions.push_back(iff);
    }

    if (false) {
      cout << "Done with section." << endl;
      char ch;
      cin >> ch;
    }
    
  }

  // Close file
  fin.close();

  // ----------------------------------------------------------------
  // Create C++ header
  
  ofstream fout;
  fout.open((cpp_prefix+".h").c_str());
  
  fout << "/*" << endl;
  for(size_t i=0;i<header.size();i++) {
    fout << header[i] << endl;
  }
  fout << "*/" << endl;
  fout << endl;
  
  for(size_t i=0;i<h_includes.size();i++) {
    fout << "#include " << h_includes[i] << endl;
  }
  fout << endl;

  fout << "extern \"C\" {" << endl;
  fout << endl;
  
  for(size_t i=0;i<classes.size();i++) {

    if_class &ifc=classes[i];

    if (ifc.is_abstract==false) {
      // Always create a void *
      fout << "void *" << underscoreify(ifc.ns) << "_create_" << ifc.name
           << "();" << endl;
      fout << endl;
      
      // Always free a void *
      fout << "void " << underscoreify(ifc.ns) << "_free_" << ifc.name
           << "(void *vp);" << endl;
      fout << endl;
    }

    for(size_t j=0;j<ifc.members.size();j++) {

      if_var &ifv=ifc.members[j];

      // Get function declaration
      if (ifv.ift.name=="bool" ||
          ifv.ift.name=="double" ||
          ifv.ift.name=="int" ||
          ifv.ift.name=="size_t") {
        if (ifv.py_name.length()>0) {
          fout << ifv.ift.name << " " << underscoreify(ifc.ns) << "_"
               << ifc.name << "_get_" << ifv.py_name << "(void *vp);" << endl;
        } else {
          fout << ifv.ift.name << " " << underscoreify(ifc.ns) << "_"
               << ifc.name << "_get_" << ifv.name << "(void *vp);" << endl;
        }
      } else {
        if (ifv.py_name.length()>0) {
          fout << "void " << underscoreify(ifc.ns) << "_"
               << ifc.name << "_get_" << ifv.py_name
               << "(void *vp, void *p_v);" << endl;
        } else {
          fout << "void " << underscoreify(ifc.ns) << "_"
               << ifc.name << "_get_" << ifv.name
               << "(void *vp, void *p_v);" << endl;
        }
      }
      fout << endl;
      
      // Set function declaration
      if (ifv.ift.name=="bool" ||
          ifv.ift.name=="double" ||
          ifv.ift.name=="int" ||
          ifv.ift.name=="size_t") {
        if (ifv.py_name.length()>0) {
          fout << "void " << underscoreify(ifc.ns) << "_"
               << ifc.name << "_set_" << ifv.py_name << "(void *vp, "
               << ifv.ift.name << " v);" << endl;
        } else {
          fout << "void " << underscoreify(ifc.ns) << "_"
               << ifc.name << "_set_" << ifv.name << "(void *vp, "
               << ifv.ift.name << " v);" << endl;
        }
      } else {
        if (ifv.py_name.length()>0) {
          fout << "void " << underscoreify(ifc.ns) << "_"
               << ifc.name << "_set_" << ifv.py_name
               << "(void *vp, void *p_v);" << endl;
        } else {
          fout << "void " << underscoreify(ifc.ns) << "_"
               << ifc.name << "_set_" << ifv.name
               << "(void *vp, void *p_v);" << endl;
        }
      }
      fout << endl;
    }
    
    for(size_t j=0;j<ifc.methods.size();j++) {
      
      if_func &iff=ifc.methods[j];
      
      // Function header
      if (iff.ret.name=="std::string") {
        fout << "const char *" << underscoreify(ifc.ns) << "_"
             << ifc.name << "_" << iff.name << "(void *vptr";
      } else if (iff.ret.name=="void" ||
                 iff.ret.name=="bool" ||
                 iff.ret.name=="double" ||
                 iff.ret.name=="int" ||
                 iff.ret.name=="size_t") {
        fout << iff.ret.name << " " << underscoreify(ifc.ns) << "_"
             << ifc.name << "_" << iff.name << "(void *vptr";
      } else {
        fout << "void *" << underscoreify(ifc.ns) << "_"
             << ifc.name << "_" << iff.name << "(void *vptr";
      }
      if (iff.args.size()>0) {
        fout << ", ";
      }
      
      for(size_t k=0;k<iff.args.size();k++) {
        if (iff.args[k].ift.suffix=="") {
          if (iff.args[k].ift.name=="std::string") {
            fout << "char *" << iff.args[k].name;
          } else {
            fout << iff.args[k].ift.name << " " << iff.args[k].name;
          }
        } else if (iff.args[k].ift.suffix=="&") {
          fout << "void *ptr_" << iff.args[k].name;
        }
        if (k!=iff.args.size()-1) {
          fout << ", ";
        }
      }
      
      fout << ");" << endl;
      fout << endl;
      
    }
    
  }
  
  for(size_t i=0;i<functions.size();i++) {
    
    if_func &iff=functions[i];
    
    // Function header
    if (iff.ret.name=="std::string") {
      fout << "const char *" << underscoreify(iff.ns) << "_"
           << iff.name << "(";
    } else if (iff.ret.name=="void" ||
               iff.ret.name=="bool" ||
               iff.ret.name=="double" ||
               iff.ret.name=="int" ||
               iff.ret.name=="size_t") {
      fout << iff.ret.name << " " << underscoreify(iff.ns) << "_"
           << iff.name << "_wrapper(";
    } else {
      fout << "void *" << underscoreify(iff.ns) << "_"
           << iff.name << "_wrapper(";
    }
    
    for(size_t k=0;k<iff.args.size();k++) {
      if (iff.args[k].ift.suffix=="") {
        if (iff.args[k].ift.name=="std::string") {
          fout << "char *" << iff.args[k].name;
        } else {
          fout << iff.args[k].ift.name << " " << iff.args[k].name;
        }
      } else if (iff.args[k].ift.suffix=="&") {
        fout << "void *ptr_" << iff.args[k].name;
      }
      if (k!=iff.args.size()-1) {
        fout << ", ";
      }
    }
    
    fout << ");" << endl;
    fout << endl;
    
  }

  // End of the extern C section
  fout << "}" << endl;
  
  fout.close();
  
  // ----------------------------------------------------------------
  // Create C++ source code 
  
  fout.open((cpp_prefix+".cpp").c_str());

  fout << "/*" << endl;
  for(size_t i=0;i<header.size();i++) {
    fout << header[i] << endl;
  }
  fout << "*/" << endl;
  fout << endl;
  
  for(size_t i=0;i<cpp_includes.size();i++) {
    fout << "#include " << cpp_includes[i] << endl;
  }
  fout << endl;

  for(size_t i=0;i<cpp_using.size();i++) {
    fout << "using namespace " << cpp_using[i] << ";" << endl;
  }
  fout << endl;

  for(size_t i=0;i<classes.size();i++) {

    if_class &ifc=classes[i];

    if (ifc.is_abstract==false) {
      // Generate code for the create pointer function
      fout << "void *" << underscoreify(ifc.ns) << "_create_" << ifc.name
           << "() {" << endl;
      fout << "  " << ifc.name << " *ptr=new " << ifc.name << ";" << endl;
      fout << "  return ptr;" << endl;
      fout << "}" << endl;
      fout << endl;
      
      // Generate code for the free pointer function
      fout << "void " << underscoreify(ifc.ns) << "_free_" << ifc.name
           << "(void *vptr) {" << endl;
      fout << "  " << ifc.name << " *ptr=(" << ifc.name << " *)vptr;" << endl;
      fout << "  delete ptr;" << endl;
      fout << "}" << endl;
      fout << endl;
    }

    for(size_t j=0;j<ifc.members.size();j++) {

      if_var &ifv=ifc.members[j];
      
      // Generate code for the get member data function
      if (ifv.ift.name=="bool" ||
          ifv.ift.name=="double" ||
          ifv.ift.name=="int" ||
          ifv.ift.name=="size_t") {
        fout << ifv.ift.name << " " << underscoreify(ifc.ns) << "_"
             << ifc.name << "_get_" << ifv.name << "(void *vptr) {" << endl;
        fout << "  " << ifc.name << " *ptr=(" << ifc.name << " *)vptr;" << endl;
        fout << "  return ptr->" << ifv.name << ";" << endl;
        fout << "}" << endl;
      } else if (ifv.ift.name=="std::string") {
        fout << "const char *" << underscoreify(ifc.ns) << "_"
             << ifc.name << "_get_" << ifv.name << "(void *vptr) {" << endl;
        fout << "  " << ifc.name << " *ptr=(" << ifc.name << " *)vptr;" << endl;
        fout << "  python_temp_string=ptr->" << ifv.name << ";" << endl;
        fout << "  return python_temp_string.c_str();" << endl;
        fout << "}" << endl;
      } else {
        fout << "void " << underscoreify(ifc.ns) << "_"
             << ifc.name << "_get_" << ifv.name
             << "(void *vptr, void *p_v) {" << endl;
        fout << "  " << ifc.name << " *ptr=(" << ifc.name << " *)vptr;" << endl;
        fout << "  " << ifv.ift.name << " *p_t=("
             << ifv.ift.name << " *)p_v;" << endl;
        fout << "  *(p_t)=ptr->" << ifv.name << ";" << endl;
        fout << "  return;" << endl;
        fout << "}" << endl;
      }
      fout << endl;
      
      // Generate code for the set member data function
      if (ifv.ift.name=="bool" ||
          ifv.ift.name=="double" ||
          ifv.ift.name=="int" ||
          ifv.ift.name=="size_t") {
        fout << "void " << underscoreify(ifc.ns) << "_"
             << ifc.name << "_set_" << ifv.name << "(void *vptr, "
             << ifv.ift.name << " v) {" << endl;
        fout << "  " << ifc.name << " *ptr=(" << ifc.name << " *)vptr;" << endl;
        fout << "  ptr->" << ifv.name << "=v;" << endl;
        fout << "  return;" << endl;
        fout << "}" << endl;
      } else {
        fout << "void " << underscoreify(ifc.ns) << "_"
             << ifc.name << "_set_" << ifv.name
             << "(void *vptr, void *p_v) {" << endl;
        fout << "  " << ifc.name << " *ptr=(" << ifc.name << " *)vptr;" << endl;
        fout << "  " << ifv.ift.name << " *p_t=("
             << ifv.ift.name << " *)p_v;" << endl;
        fout << "  ptr->" << ifv.name << "=*(p_t);" << endl;
        fout << "  return;" << endl;
        fout << "}" << endl;
      }
      fout << endl;
    }

    // Generate code for the class methods
    for(size_t j=0;j<ifc.methods.size();j++) {
      
      if_func &iff=ifc.methods[j];

      // Function header
      if (iff.ret.name=="std::string") {
        fout << "const char *" << underscoreify(ifc.ns) << "_"
             << ifc.name << "_" << iff.name << "(void *vptr";
      } else if (iff.ret.name=="void" ||
                 iff.ret.name=="bool" ||
                 iff.ret.name=="double" ||
                 iff.ret.name=="int" ||
                 iff.ret.name=="size_t") {
        fout << iff.ret.name << " " << underscoreify(ifc.ns) << "_"
             << ifc.name << "_" << iff.name << "(void *vptr";
      } else {
        fout << "void *" << underscoreify(ifc.ns) << "_"
             << ifc.name << "_" << iff.name << "(void *vptr";
      }
      if (iff.args.size()>0) {
        fout << ", ";
      }

      for(size_t k=0;k<iff.args.size();k++) {
        if (iff.args[k].ift.suffix=="") {
          if (iff.args[k].ift.name=="std::string") {
            fout << "char *" << iff.args[k].name;
          } else {
            fout << iff.args[k].ift.name << " " << iff.args[k].name;
          }
        } else if (iff.args[k].ift.suffix=="&") {
          fout << "void *ptr_" << iff.args[k].name;
        }
        if (k!=iff.args.size()-1) {
          fout << ", ";
        }
      }
      
      fout << ") {" << endl;

      // Pointer assignment for class
      fout << "  " << ifc.name << " *ptr=(" << ifc.name << " *)vptr;" << endl;
      
      // Pointer assignments for arguments
      for(size_t k=0;k<iff.args.size();k++) {
        if (iff.args[k].ift.suffix=="&") {
          fout << "  " << iff.args[k].ift.name << " *"
               << iff.args[k].name << "=("
               << iff.args[k].ift.name << " *)ptr_" << iff.args[k].name
               << ";" << endl;
        }
      }

      // Now generate code for actual function call and the
      // return statement
      if (iff.ret.name=="void") {

        fout << "  ptr->" << iff.name << "(";
        for(size_t k=0;k<iff.args.size();k++) {
          if (iff.args[k].ift.suffix=="") {
            fout << iff.args[k].name;
          } else if (iff.args[k].ift.suffix=="&") {
            fout << "*" << iff.args[k].name;
          }
          if (k!=iff.args.size()-1) {
            fout << ",";
          }
        }
        fout << ");" << endl;
        
        fout << "  return;" << endl;
        
      } else {
        
        fout << "  " << iff.ret.name << " ret=ptr->" << iff.name << "(";
        for(size_t k=0;k<iff.args.size();k++) {
          if (iff.args[k].ift.suffix=="") {
            fout << iff.args[k].name;
          } else if (iff.args[k].ift.suffix=="&") {
            fout << "*" << iff.args[k].name;
          }
          if (k!=iff.args.size()-1) {
            fout << ",";
          }
        }
        fout << ");" << endl;

        if (iff.ret.name=="std::string") {
          fout << "  python_temp_string=ret;" << endl;
          fout << "  return python_temp_string.c_str();" << endl;
        } else {
          fout << "  return ret;" << endl;
        }
      }

      // Ending function brace
      fout << "}" << endl;
      fout << endl;
    }
    
  }

  for(size_t i=0;i<functions.size();i++) {
    
    if_func &iff=functions[i];
    
    // Function header
    if (iff.ret.name=="std::string") {
      fout << "const char *" << underscoreify(iff.ns) << "_"
           << iff.name << "_wrapper(";
    } else if (iff.ret.name=="void" ||
               iff.ret.name=="bool" ||
               iff.ret.name=="double" ||
               iff.ret.name=="int" ||
               iff.ret.name=="size_t") {
      fout << iff.ret.name << " " << underscoreify(iff.ns) << "_"
           << iff.name << "_wrapper(";
    } else {
      fout << "void *" << underscoreify(iff.ns) << "_"
           << iff.name << "_wrapper(";
    }
    
    for(size_t k=0;k<iff.args.size();k++) {
      if (iff.args[k].ift.suffix=="") {
        if (iff.args[k].ift.name=="std::string") {
          fout << "char *" << iff.args[k].name;
        } else {
          fout << iff.args[k].ift.name << " " << iff.args[k].name;
        }
      } else if (iff.args[k].ift.suffix=="&") {
        fout << "void *ptr_" << iff.args[k].name;
      }
      if (k!=iff.args.size()-1) {
        fout << ", ";
      }
    }
    
    fout << ") {" << endl;
    
    // Pointer assignments for arguments
    for(size_t k=0;k<iff.args.size();k++) {
      if (iff.args[k].ift.suffix=="&") {
        fout << "  " << iff.args[k].ift.name << " *"
             << iff.args[k].name << "=("
             << iff.args[k].ift.name << " *)ptr_" << iff.args[k].name
             << ";" << endl;
      }
    }
    
    // Now generate code for actual function call and the
    // return statement
    if (iff.ret.name=="void") {
      
      fout << "  " << iff.name << "(";
      for(size_t k=0;k<iff.args.size();k++) {
        if (iff.args[k].ift.suffix=="") {
          fout << iff.args[k].name;
        } else if (iff.args[k].ift.suffix=="&") {
          fout << "*" << iff.args[k].name;
        }
        if (k!=iff.args.size()-1) {
          fout << ",";
        }
      }
      fout << ");" << endl;
      
      fout << "  return;" << endl;
      
    } else {
      
      fout << "  " << iff.ret.name << " ret=ptr->" << iff.name << "(";
      for(size_t k=0;k<iff.args.size();k++) {
        if (iff.args[k].ift.suffix=="") {
          fout << iff.args[k].name;
        } else if (iff.args[k].ift.suffix=="&") {
          fout << "*" << iff.args[k].name;
        }
        if (k!=iff.args.size()-1) {
          fout << ",";
        }
      }
      fout << ");" << endl;
      
      if (iff.ret.name=="std::string") {
        fout << "  python_temp_string=ret;" << endl;
        fout << "  return python_temp_string.c_str();" << endl;
      } else {
        fout << "  return ret;" << endl;
      }
    }
    
    fout << "}" << endl;
    fout << endl;
    
  }
  
  fout.close();
  
  // ----------------------------------------------------------------
  // Create python source code

  fout.open((py_prefix+".py").c_str());

  fout << "\"\"\"" << endl;
  for(size_t i=0;i<header.size();i++) {
    fout << header[i] << endl;
  }
  fout << "\"\"\"" << endl;
  fout << endl;
  
  fout << "import ctypes" << endl;
  fout << "from abc import abstractmethod" << endl;
  fout << "from o2sclpy.utils import force_bytes" << endl;
  fout << endl;
  
  for(size_t i=0;i<py_headers.size();i++) {
    fout << py_headers[i] << endl;
  }
  fout << endl;
  
  for(size_t i=0;i<classes.size();i++) {
    
    if_class &ifc=classes[i];
    
    if (ifc.parents.size()==0) {
      fout << "class " << ifc.name << ":" << endl;
    } else {
      fout << "class " << ifc.name << "(" << ifc.parents[0] << "):"
           << endl;
    }
    fout << "    \"\"\"" << endl;
    if (ifc.py_class_doc_pattern.length()>0) {
      string s=ifc.py_class_doc_pattern;
      while (s.find("%name%")!=std::string::npos) {
        s.replace(s.find("%name%"),6,ifc.name);
      }
      fout << "    " << s << endl;
    } else if (py_class_doc_pattern.length()>0) {
      string s=py_class_doc_pattern;
      while (s.find("%name%")!=std::string::npos) {
        s.replace(s.find("%name%"),6,ifc.name);
      }
      fout << "    " << s << endl;
    }
    fout << "    \"\"\"" << endl;
    fout << endl;
    
    // Initialize pointer
    if (ifc.parents.size()==0) {
      fout << "    _ptr=0" << endl;
      fout << "    _dll=0" << endl;
      fout << endl;
    }
    
    // Define __init__() function
    if (ifc.is_abstract) {
      fout << "    @abstractmethod" << endl;
    }
    fout << "    def __init__(self,dll):" << endl;
    fout << "        \"\"\"" << endl;
    fout << "        Init function for class " << ifc.name << " ." << endl;
    fout << "        \"\"\"" << endl;
    fout << endl;
    fout << "        f=dll." << ifc.ns << "_create_" << ifc.name << endl;
    fout << "        f.restype=ctypes.c_void_p" << endl;
    fout << "        f.argtypes=[]" << endl;
    fout << "        self._ptr=f()" << endl;
    fout << "        self._dll=dll" << endl;
    fout << "        return" << endl;
    fout << endl;
    
    // Define __del__() function
    fout << "    def __del__(self):" << endl;
    fout << "        \"\"\"" << endl;
    fout << "        Delete function for class " << ifc.name << " ." << endl;
    fout << "        \"\"\"" << endl;
    fout << endl;
    fout << "        f=self._dll." << ifc.ns << "_free_" << ifc.name << endl;
    fout << "        f.argtypes=[ctypes.c_void_p]" << endl;
    fout << "        f(self._ptr)" << endl;
    fout << "        return" << endl;
    fout << endl;

    // Define member get and set properties
    for(size_t j=0;j<ifc.members.size();j++) {
      if_var &ifv=ifc.members[j];

      // Hack, because del has a special meaning in python
      if (ifv.name=="del") ifv.name="delta";

      // Getter
      if (ifv.ift.name=="bool" ||
          ifv.ift.name=="double" ||
          ifv.ift.name=="int" ||
          ifv.ift.name=="size_t") {
        fout << "    @property" << endl;
        fout << "    def " << ifv.name << "(self):" << endl;
        fout << "        \"\"\"" << endl;
        fout << "        Getter function for " << ifc.name << "::"
             << ifv.name << " ." << endl;
        fout << "        \"\"\"" << endl;
        fout << "        func=self._dll." << ifc.ns << "_" << ifc.name
             << "_get_" << ifv.name << endl;
        fout << "        func.restype=ctypes.c_" << ifv.ift.name << endl;
        fout << "        func.argtypes=[ctypes.c_void_p]" << endl;
        fout << "        return func(self._ptr)" << endl;
      } else {
        fout << "    def get_" << ifv.name << "(self," << ifv.name
             << "):" << endl;
        fout << "        \"\"\"" << endl;
        fout << "        Getter function for " << ifc.name << "::"
             << ifv.name << " ." << endl;
        fout << "        \"\"\"" << endl;
        fout << "        func=self._dll." << ifc.ns << "_" << ifc.name
             << "_get_" << ifv.name << endl;
        
        // If it's not a string, then we don't need a return type
        if (ifv.ift.name=="std::string") {
          fout << "        func.restype=ctypes.c_char_p" << endl;
        }
        
        fout << "        func.argtypes=[ctypes.c_void_p," 
             << "ctypes.c_void_p]" << endl;
        fout << "        func(self._ptr," << ifv.name
             << "._ptr)" << endl;
        fout << "        return" << endl;
      }
      fout << endl;

      // Setter
      if (ifv.ift.name=="bool" ||
          ifv.ift.name=="double" ||
          ifv.ift.name=="int" ||
          ifv.ift.name=="size_t") {
        fout << "    @" << ifv.name << ".setter" << endl;
        fout << "    def " << ifv.name << "(self,value):" << endl;
        fout << "        \"\"\"" << endl;
        fout << "        Setter function for " << ifc.name << "::"
             << ifv.name << " ." << endl;
        fout << "        \"\"\"" << endl;
        fout << "        func=self._dll." << ifc.ns << "_" << ifc.name
             << "_set_" << ifv.name << endl;
        fout << "        func.argtypes=[ctypes.c_void_p,ctypes.c_"
             << ifv.ift.name << "]" << endl;
        fout << "        func(self._ptr,value)" << endl;
        fout << "        return" << endl;
      } else {
        fout << "    def set_" << ifv.name << "(self,value):" << endl;
        fout << "        \"\"\"" << endl;
        fout << "        Setter function for " << ifc.name << "::"
             << ifv.name << " ." << endl;
        fout << "        \"\"\"" << endl;
        fout << "        func=self._dll." << ifc.ns << "_"
             << ifc.name << "_set_" << ifv.name << endl;
        fout << "        func.argtypes=[ctypes.c_void_p,"
             << "ctypes.c_void_p]" << endl;
        fout << "        func(self._ptr,value._ptr)" << endl;
        fout << "        return" << endl;
      }
      fout << endl;
    }

    // Define methods
    for(size_t j=0;j<ifc.methods.size();j++) {

      if_func &iff=ifc.methods[j];

      // Function header
      fout << "    def " << iff.name << "(self";
      if (iff.args.size()>0) {
        fout << ",";
      }
      for(size_t k=0;k<iff.args.size();k++) {
        fout << iff.args[k].name;
        if (k!=iff.args.size()-1) {
          fout << ",";
        }
      }
      fout << "):" << endl;

      // Comment
      fout << "        \"\"\"" << endl;
      fout << "        Wrapper for " << ifc.name << "::"
           << iff.name << "() ." << endl;
      fout << "        wrapper for :ref:`o2sclp:" << ifc.name
           << "::" << iff.name << "()`." << endl;
      fout << "        \"\"\"" << endl;

      // Ctypes interface for function
      fout << "        func=self._dll." << ifc.ns << "_" << ifc.name << "_"
           << iff.name << endl;
      if (iff.ret.name!="void") {
        if (iff.ret.name=="std::string") {
          fout << "        func.restype=ctypes.c_char_p" << endl;
        } else {
          fout << "        func.restype=ctypes.c_" << iff.ret.name << endl;
        }
      }
      fout << "        func.argtypes=[ctypes.c_void_p,";
      for(size_t k=0;k<iff.args.size();k++) {
        if (iff.args[k].ift.suffix=="&") {
          fout << "ctypes.c_void_p";
        } else if (iff.args[k].ift.name=="std::string") {
          fout << "ctypes.c_char_p";
        } else {
          fout << "ctypes.c_" << iff.args[k].ift.name;
        }
        if (k!=iff.args.size()-1) {
          fout << ",";
        }
      }
      fout << "]" << endl;

      // Call C++ wrapper function
      if (iff.ret.name=="void") {
        fout << "        func(self._ptr,";
      } else {
        fout << "        ret=func(self._ptr,";
      }
      for(size_t k=0;k<iff.args.size();k++) {
        if (iff.args[k].ift.suffix=="&") {
          fout << iff.args[k].name << "._ptr";
        } else {
          fout << iff.args[k].name;
        }
        if (k!=iff.args.size()-1) fout << ",";
      }
      fout << ")" << endl;

      // Return
      if (iff.ret.name=="void") {
        fout << "        return" << endl;
      } else {
        fout << "        return ret" << endl;
      }
      fout << endl;
      
    }
    
  }

  // Define methods
  for(size_t j=0;j<functions.size();j++) {
    
    if_func &iff=functions[j];
    
    // Function header
    fout << "def " << iff.name << "(dll,";
    for(size_t k=0;k<iff.args.size();k++) {
      fout << iff.args[k].name;
      if (k!=iff.args.size()-1) {
        fout << ",";
      }
    }
    fout << "):" << endl;
    
    // Comment
    fout << "    \"\"\"" << endl;
    fout << "    Wrapper for " 
         << iff.name << "() ." << endl;
    fout << "    \"\"\"" << endl;
    
    // Perform necessary conversions
    for(size_t k=0;k<iff.args.size();k++) {
      if (iff.args[k].ift.name=="std::string") {
        fout << "    " << iff.args[k].name << "_=ctypes.c_char_p(force_bytes("
             << iff.args[k].name << "))" << endl;
      }
    }
      
    // Ctypes interface for function
    fout << "    func=dll." << iff.ns << "_" << iff.name
         << "_wrapper" << endl;
    if (iff.ret.name!="void") {
      if (iff.ret.name=="std::string") {
        fout << "    func.restype=ctypes.c_char_p" << endl;
      } else {
        fout << "    func.restype=ctypes.c_" << iff.ret.name << endl;
      }
    }
    fout << "    func.argtypes=[";
    for(size_t k=0;k<iff.args.size();k++) {
      if (iff.args[k].ift.suffix=="&") {
        fout << "ctypes.c_void_p";
      } else if (iff.args[k].ift.name=="std::string") {
        fout << "ctypes.c_char_p";
      } else {
        fout << "ctypes.c_" << iff.args[k].ift.name;
      }
      if (k!=iff.args.size()-1) {
        fout << ",";
      }
    }
    fout << "]" << endl;
    
    // Call C++ wrapper function
    if (iff.ret.name=="void") {
      fout << "    func(";
    } else {
      fout << "    ret=func(";
    }
    for(size_t k=0;k<iff.args.size();k++) {
      if (iff.args[k].ift.suffix=="&") {
        fout << iff.args[k].name << "._ptr";
      } else if (iff.args[k].ift.name=="std::string") {
        fout << iff.args[k].name << "_";
      } else {
        fout << iff.args[k].name;
      }
      if (k!=iff.args.size()-1) fout << ",";
    }
    fout << ")" << endl;
    
    // Return
    if (iff.ret.name=="void") {
      fout << "    return" << endl;
    } else {
      fout << "    return ret" << endl;
    }
    fout << endl;
    
  }
  
  fout.close();
  
  return 0;
}
