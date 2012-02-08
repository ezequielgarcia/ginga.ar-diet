/*******************************************************************************

  Copyright 2010 LIFIA - Facultad de Informatica - Univ. Nacional de La Plata

********************************************************************************

  This file is part of Ginga implementation.

    This program is free software: you can redistribute it and/or modify it 
  under the terms of the GNU General Public License as published by the Free 
  Software Foundation, either version 2 of the License.

    Ginga is distributed in the hope that it will be useful, but WITHOUT ANY 
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along with
  this program. If not, see <http://www.gnu.org/licenses/>.

********************************************************************************

  Este archivo es parte de la implementación de Ginga.

    Este programa es Software Libre: Ud. puede redistribuirlo y/o modificarlo 
  bajo los términos de la Licencia Pública General GNU como es publicada por la
  Free Software Foundation, según la versión 2 de la licencia.

    Ginga se distribuye esperando que resulte de utilidad, pero SIN NINGUNA 
  GARANTÍA; ni siquiera la garantía implícita de COMERCIALIZACIÓN o ADECUACIÓN 
  PARA ALGÚN PROPÓSITO PARTICULAR. Para más detalles, revise la Licencia Pública 
  General GNU.

    Ud. debería haber recibido una copia de la Licencia Pública General GNU 
  junto a este programa. Si no, puede verla en <http://www.gnu.org/licenses/>.

*******************************************************************************/

#include <sys/ipc.h>
#include <sys/sem.h>

#define SEMKEYPATH 	"/dev/null"  
#define SEMKEYID 	1              

extern int errno;

/*--------------------------------------------------*
 * BlitterMutexCreate()								*
 * 													*
 * Crear el semaforo para sincronizar flips			*
 * 													*
 * Retorna el identificador del semaforo o -1 si	*
 * hay error.										*
 *--------------------------------------------------*/

int BlitterMutexCreate(int keyID)
{
	key_t	semkey;
	int		semid;
	
	// Generar una clave unica para el semaforo
	semkey = ftok(SEMKEYPATH, keyID);
	if(semkey == (key_t) -1)
		return -1;

	// Crear el semaforo
	semid = semget(semkey, 1, 0666 | IPC_CREAT | IPC_EXCL);
	
	semctl(semid, 0, SETVAL, 1);

	return semid;
}

/*--------------------------------------------------*
 * BlitterMutexCreate()								*
 * 													*
 * Destruye el semaforo para sincronizar flips		*
 * 													*
 *--------------------------------------------------*/

void BlitterMutexDestroy(int semid)
{
	// Destruir el semaforo
	semid = semctl(semid, 1, IPC_RMID);

	return;
}

/*--------------------------------------------------*
 * BlitterMutexOpen()								*
 * 													*
 * Abre el semaforo para sincronizar flips			*
 * 													*
 * Retorna el identificador del semaforo o -1 si	*
 * hay error.										*
 *--------------------------------------------------*/

int BlitterMutexOpen(int keyID)
{
	key_t	semkey;
	int		semid;
	
	// Generar una clave unica para el semaforo
	semkey = ftok(SEMKEYPATH, keyID);
	if(semkey == (key_t) -1)
		return -1;

	// Abrir el semaforo
	semid = semget(semkey, 1, 0666);

	return semid;
}

/*--------------------------------------------------*
 * BlitterMutexClose(semid)							*
 * 													*
 * Cierra el semaforo para sincronizar flips		*
 * 													*
 *--------------------------------------------------*/

void BlitterMutexClose(int semid)
{
	return;
}

/*--------------------------------------------------*
 * BlitterMutexLock(semid)							*
 * 													*
 * Toma el semaforo para sincronizar flips			*
 * 													*
 *--------------------------------------------------*/

int BlitterMutexLock(int semid)
{
	struct sembuf	sops;
	int				rc;

	// Decrementa el semaforo en una unidad
	sops.sem_num = 0;
	sops.sem_op  = -1;
	sops.sem_flg = 0;

	rc = semop(semid, &sops, 1);

	return rc;
}

/*--------------------------------------------------*
 * BlitterMutexUnlock(semid)						*
 * 													*
 * Libera el semaforo para sincronizar flips		*
 * 													*
 *--------------------------------------------------*/

int BlitterMutexUnlock(int semid)
{
	struct sembuf	sops;
	int				rc;

	// Incrementa el semaforo en una unidad
	sops.sem_num = 0;
	sops.sem_op  = 1;
	sops.sem_flg = 0;

	rc = semop(semid, &sops, 1);

	return rc;
}

