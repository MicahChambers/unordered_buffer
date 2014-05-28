unordered_buffer
================

In the spirit of unordered_map, this is a probabalistic buffer. In essence you 
can insert things into the buffer, and they will replace collisions at a
probability comparable to the hit rate of the time they are replacing. So in
theory, whichever item matches a bucket gets more hits will be placed in the
bucket. 

For the current occupier of the bucket, its probability of being displaced 
is 2^(hits-contests), where hits are repeated uses of the current occupier,
and constest are the hits of colliding keys. Thus as long as there are 
less contested values, the chance of being displaced is low. We cap the 
hits-contests value to 1000, to prevent too much incumbancy. 
