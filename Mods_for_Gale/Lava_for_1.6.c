/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
** Copyright (c) 2005, Monash Cluster Computing
** All rights reserved.
** Redistribution and use in source and binary forms, with or without modification,
** are permitted provided that the following conditions are met:
**
** 		* Redistributions of source code must retain the above copyright notice,
** 			this list of conditions and the following disclaimer.
** 		* Redistributions in binary form must reproduce the above copyright
**			notice, this list of conditions and the following disclaimer in the
**			documentation and/or other materials provided with the distribution.
** 		* Neither the name of the Monash University nor the names of its contributors
**			may be used to endorse or promote products derived from this software
**			without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
** THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
** PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
** BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
** CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
** SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
** HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
** LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
** OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
**
** Contact:
*%		Louis Moresi - Louis.Moresi@sci.monash.edu.au
*%
** Contributors:
*+		Robert Turnbull
*+		Vincent Lemiale
*+		Louis Moresi
*+		David May
*+		David Stegman
*+		Mirko Velic
*+		Patrick Sunter
*+		Julian Giordani
*+
** $Id: Lava.c 610 2007-10-11 08:09:29Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>
#include <StgFEM/StgFEM.h>
#include <PICellerator/PICellerator.h>

#include "types.h"
#include "RheologyClass.h"
#include "Lava.h"
#include "ConstitutiveMatrix.h"

#include <assert.h>
#include <float.h>

/* Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
const Type Lava_Type = "Lava";

/* Private Constructor: This will accept all the virtual functions for this class as arguments. */
Lava* _Lava_New(  LAVA_DEFARGS  )
{
	Lava*					self;

	/* Call private constructor of parent - this will set virtual functions of parent and continue up the hierarchy tree. At the beginning of the tree it will allocate memory of the size of object and initialise all the memory to zero. */
	assert( _sizeOfSelf >= sizeof(Lava) );
	self = (Lava*) _Rheology_New(  RHEOLOGY_PASSARGS  );

	return self;
}

void _Lava_Init( Lava* self, FeVariable* strainRateInvField, FeVariable* temperatureField,
				 double eta0,
				 double theta,
				 double maxVis )
{
	self->strainRateInvField = strainRateInvField;
	self->temperatureField = temperatureField;
	self->eta0  = eta0;
	self->theta = theta;
	self->maxVis = maxVis;

	Rheology_SetToNonLinear( self );
}

void _Lava_Build( void* _self, void* data ){
	Lava*  self = (Lava*)_self;

	_Rheology_Build( self, data );

   Stg_Component_Build( self->strainRateInvField, data, False );
}

void _Lava_Initialise( void* _self, void* data ){
	Lava*  self = (Lava*)_self;

   _Rheology_Initialise( self, data );

   Stg_Component_Initialise( self->strainRateInvField, data, False );
}

void _Lava_Destroy( void* _self, void* data ){
	Lava*  self = (Lava*)_self;

   Stg_Component_Destroy( self->strainRateInvField, data, False );

	_Rheology_Destroy( self, data );
}


void* _Lava_DefaultNew( Name name ) {
	/* Variables set in this function */
	SizeT                                                     _sizeOfSelf = sizeof(Lava);
	Type                                                             type = Lava_Type;
	Stg_Class_DeleteFunction*                                     _delete = _Rheology_Delete;
	Stg_Class_PrintFunction*                                       _print = _Rheology_Print;
	Stg_Class_CopyFunction*                                         _copy = _Rheology_Copy;
	Stg_Component_DefaultConstructorFunction*         _defaultConstructor = _Lava_DefaultNew;
	Stg_Component_ConstructFunction*                           _construct = _Lava_AssignFromXML;
	Stg_Component_BuildFunction*                                   _build = _Lava_Build;
	Stg_Component_InitialiseFunction*                         _initialise = _Lava_Initialise;
	Stg_Component_ExecuteFunction*                               _execute = _Rheology_Execute;
	Stg_Component_DestroyFunction*                               _destroy = _Lava_Destroy;
	Rheology_ModifyConstitutiveMatrixFunction*  _modifyConstitutiveMatrix = _Lava_ModifyConstitutiveMatrix;

	/* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
	AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

	return (void*) _Lava_New(  LAVA_PASSARGS  );
}

void _Lava_AssignFromXML( void* rheology, Stg_ComponentFactory* cf, void* data ){
	Lava*  self = (Lava*)rheology;
	FeVariable    *strainRateInvField, *temperatureField;
	double refStrainRate;
	Stream* errorStream = Journal_Register( Error_Type,
											Lava_Type );

	/* Construct Parent */
	_Rheology_AssignFromXML( self, cf, data );

	/* TODO: 'Keyfallback' soon to be deprecated/updated */
	strainRateInvField = Stg_ComponentFactory_ConstructByNameWithKeyFallback(
																			 cf,
																			 self->name,
																			 (Name)"StrainRateInvariantField",
																			 (Name)"StrainRateInvariantField",
																			 FeVariable,
																			 True,
																			 data );
	temperatureField = Stg_ComponentFactory_ConstructByKey(
														   cf,
														   self->name,
														   "TemperatureField",
														   FeVariable,
														   True,
														   data );

	_Lava_Init(
			   self,
			   strainRateInvField,
			   temperatureField,
			   Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"eta0", 1.0  ),
			   Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"theta", 0.0 ),
			   Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"maxVis", 1.0e+02) );
}

void _Lava_ModifyConstitutiveMatrix(
		void*                                              rheology,
		ConstitutiveMatrix*                                constitutiveMatrix,
		MaterialPointsSwarm*                               swarm,
		Element_LocalIndex                                 lElement_I,
		MaterialPoint*                                     materialPoint,
		Coord                                              xi )
{
	Lava*	              self              = (Lava*) rheology;
	double                        strainRateInv;
	double                        temperature;
	double                        viscosity;
	double                        dTemp,Te,Tsol;
	double                        phi0,phif,phi,phi_max;
	double                        Sy;

	Stream* errorStream = Journal_Register( Error_Type,
                                                Lava_Type );

	if ( !constitutiveMatrix->previousSolutionExists )
		/* on the first ever solve, use a ref strainrate */
		{
			strainRateInv = 0.0; //self->refStrainRate;
		}
	else
		{
			/* Calculate Parameters */
			FeVariable_InterpolateWithinElement(self->strainRateInvField,
												lElement_I,xi,&strainRateInv);
		}

	/* Calculate Parameters */
	FeVariable_InterpolateFromMeshLocalCoord( self->temperatureField, ConstitutiveMatrix_GetMesh( constitutiveMatrix ), lElement_I, xi, &temperature );

	/* Yield Stress: Miyamoto & Sasaki, JGR, 1998 */
	Sy = pow(10.0,11.59-0.0089*(temperature-273.0));

	if( strainRateInv < Sy/self->maxVis ) {
		viscosity = self->maxVis;
	}
	else {
		/* Calculate New Viscosity */
		Te=1373.0;
		Tsol=600.0;
		dTemp = Te-temperature;

		phi0 = 0.0;
		phif = 0.5*(Te-Tsol)/Te;
		phi_max = phi0+phif+0.1;
		phi = (temperature>Tsol?(phi0 + phif*dTemp/(Te-Tsol)):phi_max);

		viscosity = (dTemp>0.0?(self->eta0*pow((1.0-phi/phi_max),-2.5)*exp(-self->theta*dTemp)):self->eta0);
		//maxVis = 1.0e+03*self->eta0;
		if(viscosity > self->maxVis) viscosity = self->maxVis;
	}

	ConstitutiveMatrix_SetIsotropicViscosity( constitutiveMatrix, viscosity );

}
