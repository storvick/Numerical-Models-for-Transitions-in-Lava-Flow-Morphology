Lava class is a modification of FrankKamenetskii class.
Follow how FK is defined and used in Rheology.

To add Lava class to Gale 2.0.1,

1. Copy Lava.cxx, Lava.h and Lava.meta to Gale-2_0_1/Underworld/Rheology/src.
2. Add Lava to Underworld/Rheology/src/Init.cxx just like FK.
3. Add Lava to Underworld/Rheology/src/Rheology.h.
4. Add Lava to Underworld/Rheology/src/types.h.
5. (Optional) Add Lava to MultiRheologyMaterial.meta.

To add some extra condition functions to Gale 2.0.1,

1. Copy StandardConditionsFunctions_for_2.0.cxx/h 
to StgFEM/plugins/StandardConditionFunctions/StandardConditionsFunctions.cxx/h. 
(note the file name change.)

Finally, rebuild Gale by running ./scons in the top directory of the Gale source.
==============================================================================
To add Lava class to Gale 1.6.1

Almost exactly the same except that the files named "..._for_1.6...." should be copied.

1. Copy Lava_for_1.6.c, Lava_for_1.6.h and Lava.meta to Gale-1_6__1/Underworld/Rheology/src.
2. Add Lava to Underworld/Rheology/src/Init.c just like FK.
3. Add Lava to Underworld/Rheology/src/Rheology.h.
4. Add Lava to Underworld/Rheology/src/types.h.
5. (Optional) Add Lava to MultiRheologyMaterial.meta.

To add condition functions,

1. Copy StandardConditionsFunctions_for_1.6.c/h 
to StgFEM/plugins/StandardConditionFunctions/StandardConditionsFunctions.c/h. 
(note the file name change.)

Finally, rebuild Gale by running ./scons in the top directory of the Gale source.


# Issues with building Gale-1.6 with gcc-4.9
- In case of the following error, 

    > StGermain/Base/IO/src/BinaryStream.c:165:2: error: non-void function 'BinaryStream_WriteAllProcessors' should return a value [-Wreturn-type]
    > return;
    >       ^

Maybe we should declar the function as the 'void' type?
        


