
#ifndef _eventually_define_hpp_
#define _eventually_define_hpp_

#ifndef _MSC_VER
#define NOEXCEPT noexcept
#define THROW    throw()
#else
#define NOEXCEPT
#define THROW
#endif

#endif