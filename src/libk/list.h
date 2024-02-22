#ifndef AVOCADOS_LIST_H_
#define AVOCADOS_LIST_H_

#define list_for_each(POS, HEAD)                                               \
    for (typeof(HEAD) POS = HEAD; POS != NULL; POS = POS->next)

#endif /* ! AVOCADOS_LIST_H_ */
