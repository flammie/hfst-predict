# hfst-predict

prototype for predictive texting using HFST finite-state automata.

## Making a single-automaton predictor

Any automaton that maps input strings to predicted texts is good for now, in
future we can probably also use tags for controlling a bit of behaviour.

Trivial prediction model:

```
?* 0:?::1^<5 ;
```

where 1 is the weight of prediction and 5 is the number of characters to
predict. You can replace `^<5` with infinite by `*` if the language model
is really simple (acyclic etc.), e.g. English.

Omorfi script:

```
hfst-project ../omorfi/src/generated/temporary.analyse.hfst -p input |\
    hfst-compose -2 - -1 predictor5.hfst |\
    hfst-fst2fst -f olw -o predict-omorfi.hfstol
```


