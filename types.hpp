#ifndef TYPES_HPP_XBALFVI4
#define TYPES_HPP_XBALFVI4

#include <arpa/inet.h>
#include <netinet/in.h>

#ifdef __QNX__
    using IP_REQ = ip_mreq;
#else
    using IP_REQ = ip_mreqn;
#endif

#endif /* end of include guard: TYPES_HPP_XBALFVI4 */
