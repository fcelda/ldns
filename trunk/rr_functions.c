/*
 * rr_function.c
 *
 * function that operate on specific rr types
 *
 * (c) NLnet Labs, 2004, 2005
 * See the file LICENSE for the license
 */

/*
 * these come strait from perldoc Net::DNS::RR::xxxx
 * first the read variant, then the write
 */

#include <config.h>

#include <limits.h>
#include <strings.h>

#include <ldns/rr.h>
#include <ldns/dns.h>
#include <ldns/rr_functions.h>

#include "util.h"

static ldns_rdf *
ldns_rr_function(ldns_rr_type type, ldns_rr *rr, size_t pos)
{
        if (!rr || ldns_rr_get_type(rr) != type) {
                return NULL;
        }
        return ldns_rr_rdf(rr, pos);
}

static bool
ldns_rr_set_function(ldns_rr_type type, ldns_rr *rr, ldns_rdf *rdf, size_t pos)
{
        ldns_rdf *pop;
        if (!rr || ldns_rr_get_type(rr) != type) {
                return false;
        }
        pop = ldns_rr_set_rdf(rr, rdf, pos);
        if (pop) {
                FREE(pop);
                return true;
        } else {
                return false;
        }
}

/*@ignore@*/

/* handle A / AAAA records */
ldns_rdf *
ldns_rr_address(ldns_rr *r)
{
	/* 2 types to check, cannot use the macro */
	if (!r || (ldns_rr_get_type(r) != LDNS_RR_TYPE_A &&
			ldns_rr_get_type(r) != LDNS_RR_TYPE_AAAA)) {
		return NULL;
	}
	return ldns_rr_rdf(r, 0);
}

/* write -
 * if there is a value in the rr - is is _freed_!
 */
bool
ldns_rr_set_address(ldns_rr *r, ldns_rdf *f)
{
	/* 2 types to check, cannot use the macro... */
	ldns_rdf *pop;
	if (!r || (ldns_rr_get_type(r) != LDNS_RR_TYPE_A &&
			ldns_rr_get_type(r) != LDNS_RR_TYPE_AAAA)) {
		return false;
	}
	pop = ldns_rr_set_rdf(r, f, 0);
	if (pop) {
		FREE(pop);
		return true;
	} else {
		return false;
	}
}

/* NS records */
ldns_rdf *
ldns_rr_nsdname(ldns_rr *r)
{
	return ldns_rr_function(LDNS_RR_TYPE_NS, r, 0);
}

/* MX records */
ldns_rdf *
ldns_rr_preference(ldns_rr *r)
{
	return ldns_rr_function(LDNS_RR_TYPE_MX, r, 0);
}

ldns_rdf *
ldns_rr_exchange(ldns_rr *r)
{
	return ldns_rr_function(LDNS_RR_TYPE_MX, r, 1);
}

/* RRSIG record */
ldns_rdf *
ldns_rr_typecovered(ldns_rr *r)
{
	return ldns_rr_function(LDNS_RR_TYPE_RRSIG, r, 0);
}

bool
ldns_rr_set_typecovered(ldns_rr *r, ldns_rdf *f)
{
	return ldns_rr_set_function(LDNS_RR_TYPE_RRSIG, r, f, 0);
}

ldns_rdf *
ldns_rr_algorithm(ldns_rr *r)
{
	return ldns_rr_function(LDNS_RR_TYPE_RRSIG, r, 1);
}

bool
ldns_rr_set_algorithm(ldns_rr *r, ldns_rdf *f)
{
	return ldns_rr_set_function(LDNS_RR_TYPE_RRSIG, r, f, 1);
}

ldns_rdf *
ldns_rr_labels(ldns_rr *r)
{
	return ldns_rr_function(LDNS_RR_TYPE_RRSIG, r, 2);
}
bool
ldns_rr_set_labels(ldns_rr *r, ldns_rdf *f)
{
	return ldns_rr_set_function(LDNS_RR_TYPE_RRSIG, r, f, 2);
}

ldns_rdf *
ldns_rr_origttl(ldns_rr *r)
{
	return ldns_rr_function(LDNS_RR_TYPE_RRSIG, r, 3);
}
bool
ldns_rr_set_origttl(ldns_rr *r, ldns_rdf *f)
{
	return ldns_rr_set_function(LDNS_RR_TYPE_RRSIG, r, f, 3);
}

ldns_rdf *
ldns_rr_expiration(ldns_rr *r)
{
	return ldns_rr_function(LDNS_RR_TYPE_RRSIG, r, 4);
}
bool
ldns_rr_set_expiration(ldns_rr *r, ldns_rdf *f)
{
	return ldns_rr_set_function(LDNS_RR_TYPE_RRSIG, r, f, 4);
}

ldns_rdf *
ldns_rr_inception(ldns_rr *r)
{
	return ldns_rr_function(LDNS_RR_TYPE_RRSIG, r, 5);
}
bool
ldns_rr_set_inception(ldns_rr *r, ldns_rdf *f)
{
	return ldns_rr_set_function(LDNS_RR_TYPE_RRSIG, r, f, 5);
}

ldns_rdf *
ldns_rr_keytag(ldns_rr *r)
{
	return ldns_rr_function(LDNS_RR_TYPE_RRSIG, r, 6);
}

bool
ldns_rr_set_keytag(ldns_rr *r, ldns_rdf *f)
{
	return ldns_rr_set_function(LDNS_RR_TYPE_RRSIG, r, f, 6);
}
ldns_rdf *
ldns_rr_signame(ldns_rr *r)
{
	return ldns_rr_function(LDNS_RR_TYPE_RRSIG, r, 7);
}
bool
ldns_rr_set_signame(ldns_rr *r, ldns_rdf *f)
{
	return ldns_rr_set_function(LDNS_RR_TYPE_RRSIG, r, f, 7);
}

ldns_rdf *
ldns_rr_sig(ldns_rr *r)
{
	return ldns_rr_function(LDNS_RR_TYPE_RRSIG, r, 8);
}

bool
ldns_rr_set_sig(ldns_rr *r, ldns_rdf *f)
{
	return ldns_rr_set_function(LDNS_RR_TYPE_RRSIG, r, f, 8);
}
/*@end@*/
