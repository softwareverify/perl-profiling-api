I'm adding this code for historical context.

At some point we decided it would be a good idea to create Perl versions of [Bug Validator](https://www.softwareverify.com/product/bug-validator/), [Coverage Validator](https://www.softwareverify.com/product/coverage-validator/) and [Performance Validator](https://www.softwareverify.com/product/performance-validator/).

When we inspected the existing Perl Profiling API we found that it wasn't suitable for the task, so we implemented our own Perl Profiling API.

For various reasons the Perl tools never left beta and eventually they were cancelled.

We're listing this code here because it may prove useful to someone wanting to write a Perl Profiling API. Previously this code was listed on the [Software Verify](https://www.softwareverify.com/) website.

**Perl 5.12.3**
The modifications listed here are modifications of [Perl 5.12.3](http://www.cpan.org/src/5.0/perl-5.12.3.tar.gz), hosted on CPAN.

**Files edited:**
- dump.c
		Perl_runops_debug() modified to call doProfilingCallback()
- run.c
		Perl_runops_standard() modified to call doProfilingCallback()
- win32\Makefile
		addition of profilingapi.h and profilingapi.c

**Files created:**
- profilingapi.c
- profilingapi.h

**New functions (profilingapi.c):**
- Perl_set_do_profiling();
- Perl_get_do_profiling();
- Perl_set_coverage_callback();
- Perl_set_profiling_callback();
- doProfilingCallback();	

**To build:**

These instructions are for people building with Visual Studio.  
If you are building with gcc you'll need to modify a different Makefile

cmd shell  
d:  
cd Perl-5.12.3\win32  
"c:\program files (x86)\Microsoft Visual Studio 9.0\vc\bin\vcvars32.bat"  
nmake  
nmake tests

**IMPORTANT**  
#1 If you edit **profilingapi.h** nmake won't work first time so you'll need to do nmake, then another nmake  
#2 Some of the name tests will fail (don't worry, they are meant to, nothing to do with the profiling API)




