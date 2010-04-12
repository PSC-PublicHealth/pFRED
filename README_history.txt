day [0-16383 --> 45 yrs.] (14 bits)
type [0-15] (4 bits)
strain ID [0-16383] (14 bits)
value (32 bits)

Infection:
(per-strain)
- infector
- infected place id
- status
- mutated to another strain

strain ID: 0-16383 (14 bits)
Person ID (infector): 4 billion people (32 bits)
place ID: [4 billion]  (32 bits) --> must be at least as many as people...
strain status: 4 states (2 bits)
 'R'=recovered
 'I'=symptomatic
 'E'= ?? exposed ??
 'i'=infectious
 
