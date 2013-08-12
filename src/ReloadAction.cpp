/*
 * ReloadAction.cpp
 *
 *  Created on: 14/11/2012
 *      Author: fernancoder
 */

#include "ReloadAction.h"

ReloadAction *ReloadAction::createHandler()
{
	return new ReloadAction();
}

void ReloadAction::execute()
{
	//Cierra la conexión
	//this->closeConnection();	//No hace falta es un pre-start
}

