#pragma warning( disable : 4244 )
#pragma warning( disable : 4237 )
#pragma warning( disable : 4305 )

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "cmdlib.h"
#include "scriplib.h"
#include "mathlib.h"
#include "make_mdl.h"

static int totalframes = 0;
static float totalseconds = 0;

static byte* pData;
static byte* pStart;
static studiohdr_s* phdr;

static int gnumnorms;
static byte gbonenorms[MDL_MAX_VERTS];
static mstudionorm_s gnorms[MDL_MAX_VERTS];

static int gnumverts;
static byte gboneverts[MDL_MAX_VERTS];
static mstudiovert_s gverts[MDL_MAX_VERTS];

static int gnumtexcoords;
static mstudiotexcoord_s gtexcoords[MDL_MAX_VERTS];

#define ALIGN(a) a = (byte *)((int)((byte *)a + 3) & ~ 3)

static hash_t util_hash_str(const char* string)
{
	hash_t i, hashkey = 0;

	for (i = 0; string[i]; i++)
		hashkey = (hashkey + i) * 37 + tolower(string[i]);
	return hashkey;
}

static void WriteBoneInfo()
{
	int i, j;
	mstudiobone_s* pbone;

	pbone = (mstudiobone_s*)pData;
	phdr->numbones = numbones;
	phdr->ofsbones = (pData - pStart);

	for (i = 0; i < numbones; i++)
	{
		strcpy(pbone[i].name, bonetable[i].name);
		pbone[i].hash = util_hash_str(pbone[i].name);

		pbone[i].parent = bonetable[i].parent;
		pbone[i].value[0] = bonetable[i].pos[0];
		pbone[i].value[1] = bonetable[i].pos[1];
		pbone[i].value[2] = bonetable[i].pos[2];
		pbone[i].value[3] = bonetable[i].rot[0];
		pbone[i].value[4] = bonetable[i].rot[1];
		pbone[i].value[5] = bonetable[i].rot[2];
		pbone[i].scale[0] = bonetable[i].posscale[0];
		pbone[i].scale[1] = bonetable[i].posscale[1];
		pbone[i].scale[2] = bonetable[i].posscale[2];
		pbone[i].scale[3] = bonetable[i].rotscale[0];
		pbone[i].scale[4] = bonetable[i].rotscale[1];
		pbone[i].scale[5] = bonetable[i].rotscale[2];
		pbone[i].mins[0] = bonetable[i].bmin[0];
		pbone[i].mins[1] = bonetable[i].bmin[1];
		pbone[i].mins[2] = bonetable[i].bmin[2];
		pbone[i].maxs[0] = bonetable[i].bmax[0];
		pbone[i].maxs[1] = bonetable[i].bmax[1];
		pbone[i].maxs[2] = bonetable[i].bmax[2];

		pbone[i].hitbox = 0;
		pbone[i].is_gait = false;
		for (j = 0; j < gnumgaitbones; j++)
		{
			if (strcmp(bonetable[i].name, ggaitbonenames[j]))
				continue;

			pbone[i].is_gait = true;
			break;
		}

		for (j = 0; j < gnumhitbones; j++)
		{
			if (strcmp(bonetable[i].name, ghitbonenames[j]))
				continue;

			pbone[i].hitbox = ghitboneidx[j];
			break;
		}
	}

	pData += numbones * sizeof(mstudiobone_s);
	ALIGN(pData);
}

static void WriteSequenceInfo()
{
	int i;
	mstudioseqdesc_s* pseqdesc;

	pseqdesc = (mstudioseqdesc_s*)pData;
	phdr->numseq = numseq;
	phdr->ofsseq = (pData - pStart);
	pData += numseq * sizeof(mstudioseqdesc_s);

	for (i = 0; i < numseq; i++, pseqdesc++)
	{
		strncpy(pseqdesc->name, sequence[i].name, sizeof(pseqdesc->name) - 1);
		pseqdesc->hash = util_hash_str(pseqdesc->name);

		pseqdesc->numframes = sequence[i].numframes;

		VectorCopy(sequence[i].bmin, pseqdesc->bbmin);
		VectorCopy(sequence[i].bmax, pseqdesc->bbmax);

		pseqdesc->ofsframes = sequence[i].animindex;

		totalframes += sequence[i].numframes;
		totalseconds += sequence[i].numframes / sequence[i].fps;
	}
}

static byte* WriteAnimations(byte* pData, byte* pStart, int group)
{
	int i, j, k, q, n;
	mstudioanim_s* panim;
	mstudioanimvalue_s* panimvalue;

	for (i = 0; i < numseq; i++)
	{
		if (sequence[i].seqgroup == group)
		{
			panim = (mstudioanim_s*)pData;
			sequence[i].animindex = (pData - pStart);
			pData += sequence[i].numblends * numbones * sizeof(mstudioanim_s);
			ALIGN(pData);

			panimvalue = (mstudioanimvalue_s*)pData;
			for (q = 0; q < sequence[i].numblends; q++)
			{
				for (j = 0; j < numbones; j++)
				{
					for (k = 0; k < 6; k++)
					{
						if (sequence[i].panim[q]->numanim[j][k] == 0)
							panim->offset[k] = 0;
						else
						{
							panim->offset[k] = ((byte*)panimvalue - (byte*)panim);
							for (n = 0; n < sequence[i].panim[q]->numanim[j][k]; n++)
							{
								panimvalue->value = sequence[i].panim[q]->anim[j][k][n].value;
								panimvalue++;
							}
						}
					}
					if (((byte*)panimvalue - (byte*)panim) > 65535)
						Error("sequence \"%s\" is greate than 64K\n", sequence[i].name);
					panim++;
				}
			}

			pData = (byte*)panimvalue;
			ALIGN(pData);
		}
	}
	return pData;
}

static void WriteTextures()
{
	int i, j;
	short* pref;

	phdr->numtextures = gnumtextures;
	phdr->ofstextures = (pData - pStart);

	for (i = 0; i < gnumtextures; i++)
		gtexture[i].hash = util_hash_str(gtexture[i].name);

	memcpy(pData, gtexture, gnumtextures * sizeof(mstudiotexture_s));
	pData += gnumtextures * sizeof(mstudiotexture_s);
	ALIGN(pData);

	phdr->numskins = numskinfamilies;
	phdr->ofsskins = (pData - pStart);

	pref = (short*)pData;
	for (i = 0; i < numskinfamilies; i++)
	{
		for (j = 0; j < gnumtextures; j++)
		{
			*pref = skinref[i][j];
			pref++;
		}
	}

	pData = (byte*)pref;
	ALIGN(pData);
}

static int Emit_Vertex(vec3_t v, int bone)
{
	int i;

	v[0] = (int)(v[0] * 100) / 100.0;
	v[1] = (int)(v[1] * 100) / 100.0;
	v[2] = (int)(v[2] * 100) / 100.0;

	for (i = 0; i < gnumverts; i++)
	{
		if (gboneverts[i] == bone && gverts[i].v[0] == v[0] && gverts[i].v[1] == v[1] && gverts[i].v[2] == v[2])
			return i;
	}

	if (i == MDL_MAX_VERTS)
		Error("Too many verts in model\n");

	gboneverts[i] = bone;
	gverts[i].v[0] = v[0];
	gverts[i].v[1] = v[1];
	gverts[i].v[2] = v[2];
	return gnumverts++;
}

static int Emit_Norms(vec3_t norm, int bone)
{
	int i;

	norm[0] = (int)(norm[0] * 1000000) / 1000000.0;
	norm[1] = (int)(norm[1] * 1000000) / 1000000.0;
	norm[2] = (int)(norm[2] * 1000000) / 1000000.0;

	for (i = 0; i < gnumnorms; i++)
	{
		if (gbonenorms[i] == bone && gnorms[i].norm[0] == norm[0] && gnorms[i].norm[1] == norm[1] && gnorms[i].norm[2] == norm[2])
			return i;
	}

	if (i == MDL_MAX_VERTS)
		Error("Too many normals in model\n");

	gbonenorms[i] = bone;
	gnorms[i].norm[0] = norm[0];
	gnorms[i].norm[1] = norm[1];
	gnorms[i].norm[2] = norm[2];
	return gnumnorms++;
}

static int Emit_Texcoords(float u, float v)
{
	int i;

	u = (int)(u * 1000000) / 1000000.0;
	v = (int)(v * 1000000) / 1000000.0;

	for (i = 0; i < gnumtexcoords; i++)
	{
		if (gtexcoords[i].uv[0] == u && gtexcoords[i].uv[1] == v)
			return i;
	}

	if (i == MDL_MAX_VERTS)
		Error("Too many texcoords in model\n");

	gtexcoords[i].uv[0] = u;
	gtexcoords[i].uv[1] = v;
	return gnumtexcoords++;
}

static void WriteModel()
{
	word* cmd;
	s_mesh_t* msh;
	s_model_t* mod;
	mstudiomesh_s* pmesh;
	s_trianglevert_t* tri;
	mstudiomodel_s* pmodel;
	mstudiobodyparts_s* pbodypart;
	int i, j, k, m, v, addmodels, addmeshes, cntmesh, cnttris;

	pbodypart = (mstudiobodyparts_s*)pData;
	phdr->numbodyparts = numbodyparts;
	phdr->ofsbodyparts = (pData - pStart);
	pData += numbodyparts * sizeof(mstudiobodyparts_s);

	pmodel = (mstudiomodel_s*)pData;
	phdr->nummodels = nummodels;
	phdr->ofsmodels = (pData - pStart);
	pData += nummodels * sizeof(mstudiomodel_s);

	cntmesh = cnttris = addmodels = addmeshes = 0;
	for (i = 0; i < numbodyparts; i++)
	{
		for (j = 0; j < bodypart[i].nummodels; j++)
		{
			cntmesh += bodypart[i].pmodel[j]->nummesh;
			if (cntmesh > MDL_MAX_MESHES)
				Error("Too many meshes in model\n");
		}
	}

	pmesh = (mstudiomesh_s*)pData;
	phdr->nummeshes = cntmesh;
	phdr->ofsmeshes = (pData - pStart);
	pData += cntmesh * sizeof(mstudiomesh_s);
	ALIGN(pData);

	if (numbodyparts > MDL_MAX_GROUPS)
		Error("Too many groups in model\n");

	for (i = 0; i < numbodyparts; i++)
	{
		if (bodypart[i].nummodels > MDL_MAX_BODIES)
			Error("Too many bodies in model\n");

		strncpy(pbodypart[i].name, bodypart[i].name, sizeof(pbodypart[i].name) - 1);
		pbodypart[i].hash = util_hash_str(pbodypart[i].name);

		pbodypart[i].nummodels = bodypart[i].nummodels;
		pbodypart[i].start_model = addmodels;

		for (j = 0; j < bodypart[i].nummodels; j++)
		{
			mod = bodypart[i].pmodel[j];
			strncpy(pmodel[addmodels].name, mod->name, sizeof(pmodel[addmodels].name) - 1);
			pmodel[addmodels].hash = util_hash_str(pmodel[addmodels].name);

			pmodel[addmodels].nummeshes = mod->nummesh;
			pmodel[addmodels].start_mesh = addmeshes;

			for (k = 0; k < mod->nummesh; k++)
			{
				msh = mod->pmesh[k];
				pmesh[addmeshes].texture = msh->skinref;
				pmesh[addmeshes].numcommands = msh->numtris;
				pmesh[addmeshes].ofscommands = (pData - pStart);

				cmd = (word*)pData;
				cnttris += msh->numtris;
				pData += 3 * 3 * msh->numtris * sizeof(word);
				ALIGN(pData);

				for (m = 0; m < msh->numtris; m++)
				{
					tri = msh->triangle[m];
					for (v = 0; v < 3; v++)
					{
						*cmd++ = Emit_Norms(mod->normal[tri[v].normindex].org, mod->normal[tri[v].normindex].bone);
						*cmd++ = Emit_Vertex(mod->vert[tri[v].vertindex].org, mod->vert[tri[v].vertindex].bone);
						*cmd++ = Emit_Texcoords(tri[v].u, 1.0 - tri[v].v);
					}
				}

				addmeshes++;
			}

			addmodels++;
			if (addmodels > MDL_MAX_MODELS)
				Error("Too many models in model\n");
		}
	}

	ALIGN(pData);

	phdr->numverts = gnumverts;
	phdr->ofsverts = (pData - pStart);
	memcpy(pData, gverts, gnumverts * sizeof(mstudiovert_s));
	pData += gnumverts * sizeof(mstudiovert_s);
	ALIGN(pData);

	phdr->ofsvertbones = (pData - pStart);
	memcpy(pData, gboneverts, gnumverts * sizeof(byte));
	pData += gnumverts * sizeof(byte);
	ALIGN(pData);

	phdr->numnormals = gnumnorms;
	phdr->ofsnormals = (pData - pStart);
	memcpy(pData, gnorms, gnumnorms * sizeof(mstudionorm_s));
	pData += gnumnorms * sizeof(mstudionorm_s);
	ALIGN(pData);

	phdr->ofsnormbones = (pData - pStart);
	memcpy(pData, gbonenorms, gnumnorms * sizeof(byte));
	pData += gnumnorms * sizeof(byte);
	ALIGN(pData);

	phdr->numtexcoords = gnumtexcoords;
	phdr->ofstexcoords = (pData - pStart);
	memcpy(pData, gtexcoords, gnumtexcoords * sizeof(mstudiotexcoord_s));
	pData += gnumtexcoords * sizeof(mstudiotexcoord_s);
	ALIGN(pData);

	phdr->numtris = cnttris;

	printf("%6d meshes\n", cntmesh);
	printf("%6d groups\n", numbodyparts);
	printf("%6d models\n", nummodels);

	printf("%6d verts\n", gnumverts);
	printf("%6d norms\n", gnumnorms);
	printf("%6d texcoords\n", gnumtexcoords);

	printf("%6d tris\n", phdr->numtris);
}

#define FILEBUFFER (16 * 1024 * 1024)

void WriteFile()
{
	int total = 0;
	FILE* modelouthandle;

	pStart = kalloc(1, FILEBUFFER);
	StripExtension(outname);
	strcat(outname, ".mdl");

	printf("---------------------\n");
	printf("writing %s:\n", outname);
	modelouthandle = SafeOpenWrite(outname);

	phdr = (studiohdr_s*)pStart;
	phdr->magic = MDL_NAGIC;
	pData = (byte*)phdr + sizeof(studiohdr_s);

	WriteBoneInfo();
	printf("bones     %6d bytes (%d)\n", pData - pStart - total, numbones);
	total = pData - pStart;

	pData = WriteAnimations(pData, pStart, 0);

	WriteSequenceInfo();
	printf("sequences %6d bytes (%d frames) [%d:%02d]\n", pData - pStart - total, totalframes, (int)totalseconds / 60, (int)totalseconds % 60);
	total = pData - pStart;

	WriteModel();
	printf("models    %6d bytes\n", pData - pStart - total);
	total = pData - pStart;

	WriteTextures();
	printf("textures  %6d bytes\n", pData - pStart - total);

	printf("total     %6d\n", (int)(pData - pStart));
	SafeWrite(modelouthandle, pStart, (int)(pData - pStart));
	fclose(modelouthandle);
}