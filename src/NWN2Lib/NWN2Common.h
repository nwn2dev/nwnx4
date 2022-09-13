#ifndef _NWNX4_NWN2LIB_NWN2COMMON_H
#define _NWNX4_NWN2LIB_NWN2COMMON_H

#include <windows.h>

//
// General header file for reverse engineered NWN2 information, not specific to
// the client or server binary.
//

namespace NWN2
{

	//
	// Position list for pathing information.
	//

	struct position
	{
		float x;
		float y;
		float z;
		unsigned long idx; // 16-bits with padding perhaps
	};

	struct position_list
	{
		struct position_list *next;
		struct position_list *prev;
		position              pos; // some large array
	};

	struct somestruc
	{
		// positions sorted in ascending order from tail by y
		char                  unk[0x24];     // +0x00
		struct position_list *head;  // +0x24
		struct position_list *tail;  // +0x28
		char                  unk2[0x2c];    // +0x2c
		int                   flag;           // +0x58
	};

}

namespace NWN
{
	typedef unsigned long OBJECTID;
	typedef unsigned long PLAYERID;

	static const OBJECTID INVALIDOBJID          = 0x7F000000;

	static const unsigned long LISTTYPE_MASK    = 0x80000000;

	static const PLAYERID PLAYERID_INVALIDID    = 0xFFFFFFFE;

	struct Vector3
	{
		float x;
		float y;
		float z;
	};

	struct Matrix44
	{
		float m[4][4];
	};

	struct D3DXCOLOR
	{
		float r;
		float g;
		float b;
		float a;
	};

	struct NWN2_TintSet
	{
		D3DXCOLOR Colors[ 3 ];
	};

	C_ASSERT( sizeof( NWN2_TintSet ) == 0x30 );

	struct QuickRay
	{
		NWN::Vector3 o;
		NWN::Vector3 d;
		float R0;
		float R1;
		float R2;
		float R3;
		float R4;
		float R6;
		ULONG64 classification;
		float ii;
		float ij;
		float ik;
	};

	C_ASSERT( sizeof( QuickRay ) == 0x48 );

	template< typename T >
	struct NWN2_DynamicArrayBlittable
	{
		T * m_Array;
		int m_nAllocatedSize;
		int m_nUsedSize;
	};

	typedef enum _OBJECT_TYPE
	{
		OBJECT_TYPE_GUI                = 0x01,
		OBJECT_TYPE_TILE               = 0x02,
		OBJECT_TYPE_MODULE             = 0x03,
		OBJECT_TYPE_AREA               = 0x04,
		OBJECT_TYPE_CREATURE           = 0x05,
		OBJECT_TYPE_ITEM               = 0x06,
		OBJECT_TYPE_TRIGGER            = 0x07,
		OBJECT_TYPE_PROJECTILE         = 0x08,
		OBJECT_TYPE_PLACEABLE          = 0x09,
		OBJECT_TYPE_DOOR               = 0x0A,
		OBJECT_TYPE_AREAOFEFFECTOBJECT = 0x0B,
		OBJECT_TYPE_WAYPOINT           = 0x0C,
		OBJECT_TYPE_ENCOUNTER          = 0x0D,
		OBJECT_TYPE_STORE              = 0x0E,
		OBJECT_TYPE_PORTAL             = 0x0F,
		OBJECT_TYPE_SOUND              = 0x10,
		OBJECT_TYPE_DYNAMIC            = 0x11,
		OBJECT_TYPE_STATIC_CAMERA      = 0x12,
		OBJECT_TYPE_ENVIRONMENT_OBJECT = 0x13,
		OBJECT_TYPE_TREE               = 0x14,
		OBJECT_TYPE_LIGHT              = 0x15,
		OBJECT_TYPE_PLACED_EFFECT      = 0x16,
		OBJECT_TYPE_COUNT              = 0x17
	} OBJECT_TYPE, * POBJECT_TYPE;





	union CGameObjectFlags
	{
		unsigned __int64 Flags;
		struct
		{
			unsigned long NewPosition        : 1;
			unsigned long NewHeading         : 1;
			unsigned long NewOrientation     : 1;
			unsigned long NewScale           : 1;
			unsigned long WorldTransIsDirty  : 1;
			unsigned long Visible            : 1;
			unsigned long MarkedForDeletion  : 1;
			unsigned long Reserved1          : 1;
			unsigned long Reserved2          : 24;
			unsigned long AttachedToPoint    : 9;
			unsigned long AttachedToVia      : 9;
			unsigned long Reserved3          : 14;
		};
	};

	C_ASSERT( sizeof( CGameObjectFlags ) == 8 );

	struct CNWSCreature;
	struct CNWSArea;
	struct CNWSItem;
	struct CNWSObject;

	struct BugFix_ObjectInfo
	{
		struct CGameObject * Object;
		NWN::OBJECTID ObjectId;
	};

	struct CGameObject
	{
		void                                   * vtable[4];
		NWN::Vector3                             m_vUp;
		NWN::Vector3                             m_vHeading;
		NWN::Vector3                             m_vScale;
		union
		{
			unsigned char                        Padding0[ 0x0C ];
			USHORT                               BugFix_AISortingLevel; // Added by xp_bugfix!  For non-module/area objects.
			NWN2_DynamicArrayBlittable<BugFix_ObjectInfo> BugFix_AreaObjectList; // Added by xp_bugfix!  For area objects.
		};
		NWN::Matrix44                            m_WorldTrans;
		struct NWN2_Object                     * m_pAttachedTo;
		unsigned char                            m_AttachmentPoints[ 0x0C ];
		CGameObjectFlags                         Flags;
		unsigned long                            m_PrmaryUpdateCycle;
		unsigned long                            m_SecondaryUpdateCycle;
		NWN::OBJECTID                            m_idSelf;
		unsigned char                            m_nObjectType;
		int                                      m_nAnimation;
		float                                    m_fAnimSpeed;
		float                                    m_fAnimDuration;
		struct DialogueData                    * m_pDialogueData;
		union
		{
			unsigned char                        Padding1[ 0x8 ];

			//
			// This field maintains the update generation of the associated
			// area, or the area itself for a CNWSArea object.  The update
			// generation is incremented for an area when it removes an object
			// from the area.
			//
			// The update generation is updated for a creature to that of the
			// value of its current area when a LUO other object deletion check
			// is performed.
			//
			// Only strictly used for CNWSCreature (owned by the player object!
			// and CNWSArea at present.
			//

			unsigned long                        BugFix_UpdateGeneration; // Added by xp_bugfix!
		};

		inline
		NWN::OBJECT_TYPE
		GetObjectType(
			) const
		{
			return (NWN::OBJECT_TYPE) m_nObjectType;
		}

		inline
		NWN::OBJECTID
		GetObjectId(
			) const
		{
			return m_idSelf;
		}

		inline
		CNWSCreature *
		AsCreature(
			)
		{
			if (GetObjectType( ) != NWN::OBJECT_TYPE_CREATURE)
				return NULL;

			return (CNWSCreature *) this;
		}

		inline
		CNWSArea *
		AsArea(
			)
		{
			if (GetObjectType( ) != NWN::OBJECT_TYPE_AREA)
				return NULL;

			return (CNWSArea *) ((char *) this - ArGameObjectBaseClassOffset);
		}

		inline
		CNWSItem *
		AsItem(
			)
		{
			if (GetObjectType( ) != NWN::OBJECT_TYPE_ITEM)
				return NULL;

			return (CNWSItem *) ((char *) this - ItGameObjectBaseClassOffset);
		}

		inline
		CNWSObject *
		AsNWSObject(
			)
		{
			switch (GetObjectType( ))
			{

			case NWN::OBJECT_TYPE_AREA:
				return NULL;

			case NWN::OBJECT_TYPE_ITEM:
				return (CNWSObject *) AsItem( );

			case NWN::OBJECT_TYPE_CREATURE:
			case NWN::OBJECT_TYPE_PLACEABLE:
			case NWN::OBJECT_TYPE_STORE:
			case NWN::OBJECT_TYPE_DOOR:
			case NWN::OBJECT_TYPE_ENCOUNTER:
			case NWN::OBJECT_TYPE_TRIGGER:
			case NWN::OBJECT_TYPE_AREAOFEFFECTOBJECT:
			case NWN::OBJECT_TYPE_WAYPOINT:
			case NWN::OBJECT_TYPE_STATIC_CAMERA:
			case NWN::OBJECT_TYPE_TREE:
			case NWN::OBJECT_TYPE_LIGHT:
			case NWN::OBJECT_TYPE_TILE:
			case NWN::OBJECT_TYPE_PLACED_EFFECT:
			case NWN::OBJECT_TYPE_SOUND:
			case NWN::OBJECT_TYPE_ENVIRONMENT_OBJECT:
				return (CNWSObject *) this;

			default:
				return NULL;

			}
		}
	};

	C_ASSERT( sizeof( CGameObject ) == 0xC0 );
	C_ASSERT( offsetof( CGameObject, m_pDialogueData ) == 0xB4 );
	C_ASSERT( offsetof( CGameObject, m_idSelf ) == 0xa0);
	C_ASSERT( offsetof( CGameObject, m_pAttachedTo ) == 0x80);
	C_ASSERT( offsetof( CGameObject, m_WorldTrans ) == 0x40);
	C_ASSERT( offsetof( CGameObject, m_vUp ) == 0x10 );
	C_ASSERT( offsetof( CGameObject, m_vHeading ) == 0x1C );
	C_ASSERT( offsetof( CGameObject, m_vScale ) == 0x28 );


	enum TintSetType
	{
		CreatureTintSetBody,
		CreatureTintSetHead,
		CreatureTintSetHair,

		LastCreatureTintSet
	};


	struct CNWSCreatureStatsCore
	{
		inline
		void
		SetHeadVariation(
			__in unsigned char Variation
			)
		{
			*(unsigned char *) ((char *) this + CsHeadVariation) = Variation;
		}

		inline
		void
		SetHairVariation(
			__in unsigned char Variation
			)
		{
			*(unsigned char *) ((char *) this + CsHairVariation) = Variation;
		}

		inline
		void
		SetTailVariation(
			__in unsigned char Variation
			)
		{
			*(unsigned char *) ((char *) this + CsTailVariation) = Variation;
		}

		inline
		void
		SetWingVariation(
			__in unsigned char Variation
			)
		{
			*(unsigned char *) ((char *) this + CsWingVariation) = Variation;
		}

		inline
		void
		SetFacialHairVariation(
			__in unsigned char Variation
			)
		{
			*(unsigned char *) ((char *) this + CsFacialHairVariation) = Variation;
		}

		inline
		void
		SetTintSet(
			__in TintSetType WhichTintSet,
			__in const NWN::NWN2_TintSet * TintSet
			)
		{
			NWN::NWN2_TintSet * ObjTintSet;

			switch (WhichTintSet)
			{

			case CreatureTintSetBody:
				ObjTintSet = (NWN::NWN2_TintSet *) ((char *) this + CsTint);
				break;

			case CreatureTintSetHead:
				ObjTintSet = (NWN::NWN2_TintSet *) ((char *) this + CsHeadTint);
				break;

			case CreatureTintSetHair:
				ObjTintSet = (NWN::NWN2_TintSet *) ((char *) this + CsHairTint);
				break;

			default:
				return;

			}

			memcpy( ObjTintSet, TintSet, sizeof( *TintSet ) );
		}

		inline
		void
		SetRace(
			__in unsigned short Race
			)
		{
			*(unsigned short *) ((char *) this + CsRace) = Race;
		}
	};

	struct CNWSCreature : public CGameObject
	{

		inline
		CNWSCreatureStatsCore *
		GetStats(
			)
		{
			return *(CNWSCreatureStatsCore **) ((char *) this + CrStats);
		}

		inline
		BOOL
		GetIsPlayerCharacter(
			)
		{
			return *(BOOL *) ((char *) this + CrPlayerCharacter);
		}

		inline
		NWN::PLAYERID
		GetControllingPlayerId(
			)
		{
			return *(NWN::PLAYERID *) ((char *) this + CrControllingPlayerId);
		}

		inline
		void
		SetHeadVariation(
			__in unsigned char Variation
			)
		{
			*(unsigned char *) ((char *) this + CrAppearance + CaHeadVariation) = Variation;

			GetStats( )->SetHeadVariation( Variation );
		}

		inline
		void
		SetHairVariation(
			__in unsigned char Variation
			)
		{
			*(unsigned char *) ((char *) this + CrAppearance + CaHairVariation) = Variation;

			GetStats( )->SetHairVariation( Variation );
		}

		inline
		void
		SetTailVariation(
			__in unsigned char Variation
			)
		{
			*(unsigned char *) ((char *) this + CrAppearance + CaTailVariation) = Variation;

			GetStats( )->SetTailVariation( Variation );
		}

		inline
		void
		SetWingVariation(
			__in unsigned char Variation
			)
		{
			*(unsigned char *) ((char *) this + CrAppearance + CaWingVariation) = Variation;

			GetStats( )->SetWingVariation( Variation );
		}

		inline
		void
		SetFacialHairVariation(
			__in unsigned char Variation
			)
		{
			*(unsigned char *) ((char *) this + CrAppearance + CaFacialHairVariation) = Variation;

			GetStats( )->SetFacialHairVariation( Variation );
		}

		inline
		void
		SetTintSet(
			__in TintSetType WhichTintSet,
			__in const NWN::NWN2_TintSet * TintSet
			)
		{
			NWN::NWN2_TintSet * ObjTintSet;

			switch (WhichTintSet)
			{

			case CreatureTintSetBody:
				ObjTintSet = (NWN::NWN2_TintSet *) ((char *) this + CrAppearance + CaTint);
				break;

			case CreatureTintSetHead:
				ObjTintSet = (NWN::NWN2_TintSet *) ((char *) this + CrAppearance + CaHeadTint);
				break;

			case CreatureTintSetHair:
				ObjTintSet = (NWN::NWN2_TintSet *) ((char *) this + CrAppearance + CaHairTint);
				break;

			default:
				return;

			}

			memcpy( ObjTintSet, TintSet, sizeof( *TintSet ) );

			GetStats( )->SetTintSet( WhichTintSet, TintSet );
		}

		inline
		void
		SetRace(
			__in unsigned short Race
			)
		{
			GetStats( )->SetRace( Race );
		}
	};

	C_ASSERT( offsetof( CNWSCreature, vtable ) == 0 );

	//
	// 12 bytes of padding overlaid with xp_bugfix context state that are at
	// the end of the CNWSObject structure.
	//
	// Note that this is NOT usable for area objects which do not derive from
	// CNWSObject.
	//

	union BugFix_CNWSObject
	{
		unsigned char Padding[ 0x0C ];
		struct
		{
			NWN::OBJECTID LastUpdateAreaObjectId;  // Only used for CNWSCreature
		};
	};

	C_ASSERT( sizeof( BugFix_CNWSObject) == 12 );

	struct CExoString
	{
		char * m_sString;
		ULONG m_nBufferLength;
	};

	C_ASSERT( sizeof( CExoString ) == 0x08 );

	struct CNWSObject : public CGameObject
	{
		inline
		int
		GetAILevel(
			)
		{
			return *(int *) ((char *) this + SoAILevel);
		}

		inline
		void
		SetAILevel(
			__in int AILevel
			)
		{
			*(int *) ((char *) this + SoAILevel) = AILevel;
		}

		inline
		NWN::OBJECTID
		GetDialogOwner(
			)
		{
			return *(NWN::OBJECTID *) ((char *) this + SoDialogOwner);
		}

		inline
		void
		SetDialogOwner(
			__in NWN::OBJECTID DialogOwner
			)
		{
			*(NWN::OBJECTID *) ((char *) this + SoDialogOwner) = DialogOwner;
		}

		inline
		NWN::OBJECTID
		GetArea(
			)
		{
			return *(NWN::OBJECTID *) ((char *) this + SoArea);
		}

		inline
		void
		SetArea(
			__in NWN::OBJECTID AreaObjectId
			)
		{
			*(NWN::OBJECTID *) ((char *) this + SoArea) = AreaObjectId;
		}

		//
		// Return a pointer to 12 bytes of xp_bugfix context that are overlaid
		// with unused padding in the game defined structure.
		//

		inline
		NWN::BugFix_CNWSObject *
		GetBugFix_CNWSObject(
			)
		{
			return (NWN::BugFix_CNWSObject *) ((char *) this + SoBugFixNWSObject);
		}

		inline
		const NWN::CExoString *
		GetFirstName(
			)
		{
			return (const NWN::CExoString *) ((char *) this + SoFirstName);
		}

		inline
		const NWN::CExoString *
		GetLastName(
			)
		{
			return (const NWN::CExoString* ) ((char *) this + SoLastName);
		}

	};

	C_ASSERT( offsetof( CNWSObject, vtable ) == 0 );

	struct CNWSItem_Bases
	{
		unsigned char ItemObject__BaseClass_Data[ ItGameObjectBaseClassOffset ];
	};

	struct CNWSItem : public CNWSItem_Bases,
	                  public CGameObject
	{
		inline
		CGameObject *
		AsGameObject(
			)
		{
			return (CGameObject *) (((char *) this) + ItGameObjectBaseClassOffset );
		}

		inline
		BOOL
		GetShouldUpdateEffects(
			)
		{
			return *(int *) ((char *) this + ItShouldUpdateEffects);
		}

		inline
		void
		SetShouldUpdateEffects(
			__in BOOL Enable
			)
		{
			*(int *) ((char *) this + ItShouldUpdateEffects) = Enable;
		}
	};

	C_ASSERT( offsetof( CNWSItem, vtable ) == ItGameObjectBaseClassOffset );


	struct CNWSArea_Bases
	{
		unsigned char AreaObject__BaseClass_Data[ ArGameObjectBaseClassOffset ];
	};

	struct CNWSArea : public CNWSArea_Bases,
	                  public CGameObject
	{
		inline
		NWN2_DynamicArrayBlittable<NWN::OBJECTID> &
		GetObjects (
			)
		{
			return *( NWN2_DynamicArrayBlittable<NWN::OBJECTID> *) ((char *) this + ArObjects);
		}
	};

	C_ASSERT( offsetof( CNWSArea, vtable ) == ArGameObjectBaseClassOffset );

	struct CNetLayer
	{
		static const PLAYERID PLAYERID_INVALIDID = NWN::PLAYERID_INVALIDID;
	};

	struct CGameObjectArrayNode
	{
		NWN::OBJECTID          m_objectId;
		CGameObject          * m_objectPtr;
		CGameObjectArrayNode * m_nextNode;
	};

	C_ASSERT( sizeof( CGameObjectArrayNode ) == 0x0c );

	struct CGameObjectArray
	{
		CGameObjectArrayNode * m_pArray[ 8192 ];
		NWN::OBJECTID m_nNextObjectArrayID[ 2 ];
		NWN::OBJECTID m_nNextCharArrayID[ 2 ] ;
		BOOL m_bClientMode;
	};

	C_ASSERT( sizeof( CGameObjectArray ) == 0x8014 );

	struct CExoLinkedListNode
	{
		struct CExoLinkedListNode * pPrev;
		struct CExoLinkedListNode * pNext;
		void * pObject;
	};

	C_ASSERT( sizeof( CExoLinkedListNode ) == 0x0c );

	template< typename T >
	struct CExoLinkedList
	{
		CExoLinkedListNode * pHead;
		CExoLinkedListNode * pTail;
		ULONG m_nCount;
	};

	struct CServerAIList
	{
		NWN2_DynamicArrayBlittable< CGameObject * > m_aoGameObjects; // By default is an array of NWN::OBJECTID, but patched to be CGameObject *
		int m_nPosition;
		int m_nLastFramePosition;
	};

	C_ASSERT( sizeof( CServerAIList ) == 0x14 );

	struct CGameEffect
	{
		ULONG64 m_nID;
		int m_nType;
		USHORT m_nSubType;
		float m_fDuration;
		ULONG m_nExpiryCalendarDay;
		ULONG m_nExpiryTimeOfDay;
		NWN::OBJECTID m_oidCreator;
		ULONG m_nSpellId;
		BOOL m_bExpose;
		BOOL m_bShowIcon;
		int m_nCasterLevel;
		struct CGameEffect * m_pLinkLeft;
		struct CGameEffect * m_pLinkRight;
		int m_nNumIntegers;
		int m_nNumFloats;
		int * m_nParamInteger;
		float * m_nParamFloat;
		CExoString m_sParamString[6];
		NWN::OBJECTID m_oidParamObjectID[4];
		struct CVirtualMachineScript * m_pScriptSituations[4];
		BOOL m_bSkipOnLoad;
	};

	C_ASSERT( sizeof( CGameEffect ) == 0xa0 );

	enum
	{
		EFFECT_ITEMPROPERTY = 0x5B,
		EFFECT_DAMAGE_RESISTANCE = 0x02,
	};

	enum
	{
		DURATION_TYPE_TEMPORARY = 0x01,

		DURATION_TYPE_MASK = 0x07,
	};

	struct CServerAIMaster
	{
		struct CServerExoAppInternal * m_pExoAppInternal;
		CServerAIList m_aGameAIList;
		CExoLinkedList< int > m_lEventQueue; // CServerAIEventNode
		void * m_pGameEffectApplierRemover;
		void * m_pItemPropertyApplierRemover;
		CExoString m_sScriptsRun;
		BOOL m_bPlotPathRun;
		BOOL m_bGridPathRun;
		BOOL m_bUpdateVisibleListRun;
		BOOL m_bBroadcastAOORun;
		BOOL m_bTrapCheckRun;
		int m_nLastActionRun;
		int m_nLastInactiveAI;
		void * m_pAttackRepAdj;
		void * m_pTheftRepAdj;
		void * m_pKillRepAdj;
		void * m_pHelpRepAdj;
	};

	C_ASSERT( sizeof( CServerAIMaster ) == 0x60 );

	struct CServerExoAppInternal
	{
		UCHAR Unknown[ 0x1006C ];
		CServerAIMaster *AIMaster;
	};

	C_ASSERT( offsetof( CServerExoAppInternal, AIMaster) == 0x1006C );

	struct CServerExoApp
	{
		void * Vtable;
		CServerExoAppInternal * m_pcExoAppInternal;
		CGameObjectArray * m_pGameObjArray;
	};

	C_ASSERT( sizeof( CServerExoApp ) == 0x0c );

	struct CAppManager
	{
		void * m_pClientExoApp;
		CServerExoApp * m_pServerExoApp;
		void * m_pClientObjectTableManager;
		void * m_pServerObjectTableManager;
		void * m_pReentrantServerStats;
		BOOL m_bDungeonMasterEXERunning;
		ULONG m_nApplicationId;
		ULONG m_nApplicationFrame;
		BOOL m_bMultiplayerEnabled;
		BOOL m_bWasPaused;
	};

	struct CNWSPlayer;

	struct CLastUpdateObject
	{
		UCHAR Reserved0[ 0x7bc ];
		NWN::OBJECTID m_nId;
		UCHAR Reserved1[ 0x9d ];
		UCHAR PlayerPtrHigh;
		UCHAR PlayerPtrMid[ 2 ];
		UCHAR Reserved2[ 0x12d ];
		UCHAR PlayerPtrLow;
		UCHAR AvailableForUse[ 2 ];
		UCHAR Reserved3[ 0x64 ];

		//
		// Retrieve the xp_bugfix CNWSPlayer backlink that is pakced into spare
		// padding in the CLastUpdateObject structure.
		//

		inline
		CNWSPlayer *
		GetPlayer(
			)
		{
			ULONG_PTR PlayerPtr;

			PlayerPtr = 0;
			PlayerPtr |= this->PlayerPtrHigh;
			PlayerPtr <<= 8;
			PlayerPtr |= this->PlayerPtrMid[1];
			PlayerPtr <<= 8;
			PlayerPtr |= this->PlayerPtrMid[0];
			PlayerPtr <<= 8;
			PlayerPtr |= this->PlayerPtrLow;

			return (CNWSPlayer *) (PlayerPtr);
		}

		inline
		void
		SetPlayer(
			__in CNWSPlayer * Player
			)
		{
			ULONG_PTR PlayerPtr;

			PlayerPtr = (ULONG_PTR) Player;

			this->PlayerPtrLow = (UCHAR) (PlayerPtr & 0xFF);
			PlayerPtr >>= 8;
			this->PlayerPtrMid[0] = (UCHAR) (PlayerPtr & 0xFF);
			PlayerPtr >>= 8;
			this->PlayerPtrMid[1] = (UCHAR) (PlayerPtr & 0xFF);
			PlayerPtr >>= 8;
			this->PlayerPtrHigh = (UCHAR) (PlayerPtr & 0xFF);
		}
	};

	C_ASSERT( offsetof( CLastUpdateObject, m_nId ) == 0x7bc );
	C_ASSERT( offsetof( CLastUpdateObject, Reserved1 ) == 0x7c0 );
	C_ASSERT( offsetof( CLastUpdateObject, PlayerPtrHigh ) == 0x85d );
	C_ASSERT( offsetof( CLastUpdateObject, PlayerPtrMid ) == 0x85e );
	C_ASSERT( offsetof( CLastUpdateObject, Reserved2 ) == 0x860 );
	C_ASSERT( offsetof( CLastUpdateObject, PlayerPtrLow ) == 0x98d );
	C_ASSERT( offsetof( CLastUpdateObject, AvailableForUse ) == 0x98e );
	C_ASSERT( offsetof( CLastUpdateObject, Reserved3 ) == 0x990 );
	C_ASSERT( sizeof( CLastUpdateObject ) == 0x9f4 );

	struct CLastPartyUpdateObject
	{
		UCHAR Reserved0[ 0x9c ];
	};

	C_ASSERT( sizeof( CLastPartyUpdateObject ) == 0x9c );

	struct CNWSPlayerLUOLastAreaUpdate;
	struct CNWSPlayerJournalQuest;
	struct CNWSPlayerStoreGUI;
	struct CNWSPlayerInventoryGUI;
	struct CNWSPlayerRosterGUI;
	struct CNWSPlayerLastUpdateObject;
	struct CNWSPlayerCharSheetGUI;
	struct NWN2_CreatureStatePref;

	struct CResRef
	{
		UCHAR m_resRef[32];
	};

	struct BugFix_LUOMap
	{
		NWN::OBJECTID       ObjectId;
		CLastUpdateObject * LUO;
	};

	struct CNWSPlayer
	{
		void                                   * vtable;
		ULONG                                    m_nPlayerID;
		int                                      m_nLanguage;
		CExoLinkedList<CLastUpdateObject>      * m_pActiveObjectsLastUpdate;
		CExoLinkedList<CLastPartyUpdateObject> * m_pActivePartyObjectsLastUpdate;
		CNWSPlayerLUOLastAreaUpdate            * m_pAreaLUO;
		int                                      m_nAreaTransitionBMP;
		CExoString                               m_sAreaTransitionName;
		BOOL                                     m_bFloatyEffects;
		int                                      m_nAreas;
		PULONG                                   m_pAreas;
		UCHAR                                    m_nLoginState;
		NWN::OBJECTID                            m_oidNWSObject;
		ULONG64                                  m_nLastUpdatedTime;
		NWN::OBJECTID                            m_oidLastObjectControlled;
		NWN::OBJECTID                            m_oidPCObject;
		BOOL                                     m_bIsPrimaryPlayer;
		BOOL                                     m_bFromSaveGame;
		BOOL                                     m_bFromTURD;
		ULONG                                    m_nPlayerListIndex;
		BOOL                                     m_bPlayModuleListingCharacters;
		CNWSPlayerJournalQuest                 * m_pJournalQuest;
		CNWSPlayerStoreGUI                     * m_pStoreGUI;
		CNWSPlayerInventoryGUI                 * m_pInventoryGUI;
		CNWSPlayerInventoryGUI                 * m_pOtherInventoryGUI;
		CNWSPlayerRosterGUI                    * m_pRosterGUI;
		NWN2_DynamicArrayBlittable<CNWSPlayerLastUpdateObject *> m_lstPlayerLUO;
		NWN2_DynamicArrayBlittable<CNWSPlayerCharSheetGUI *> m_lstCharSheetGUI;
		int                                      m_bAlwaysRun;
		UCHAR                                    m_nCharacterType;
		CResRef                                  m_resFileName;
		BOOL                                     m_bCommunityNameAuthroized;
		BOOL                                     m_bModuleInfoSucceeded;
		ULONG                                    m_nIFOCharacterIndex;
		BOOL                                     m_bCutsceneState;
		NWN::OBJECTID                            m_oidCurTargetObject;
		NWN::OBJECTID                            m_oidCreatureExamineTarget;
		BOOL                                     m_bLoadedFromPInfo;
		bool                                     m_bOnPCLoadedFired;
		bool                                     m_bReceivedModuleRunning;
		bool                                     m_bInMPCutscene;
		bool                                     m_bInPartyChat;
		CHAR                                     m_pLastModuleID[ 32 ] ;
		bool                                     m_bModuleInitialized;
		BOOL                                     m_bNeedsInventoryUpdate;
		NWN2_DynamicArrayBlittable<NWN2_CreatureStatePref *> m_lstCreatureStatePreferences;
		NWN2_CreatureStatePref                 * m_pOwnedCreatureStatePreferences;
		NWN2_DynamicArrayBlittable<NWN::OBJECTID> m_lstSelectionGroup;

		union
		{
			ULONG                                Padding0;
			NWN2_DynamicArrayBlittable<BugFix_LUOMap> * BugFix_LUOMap;
		};

	};

	C_ASSERT( offsetof( CNWSPlayer, m_pActiveObjectsLastUpdate ) == 0x0c );
	C_ASSERT( offsetof( CNWSPlayer, m_nLoginState ) == 0x30 );
	C_ASSERT( offsetof( CNWSPlayer, m_nPlayerListIndex ) == 0x54 );
	C_ASSERT( offsetof( CNWSPlayer, m_oidCurTargetObject ) == 0xc0 );
	C_ASSERT( offsetof( CNWSPlayer, m_pLastModuleID ) == 0xd0 );
	C_ASSERT( offsetof( CNWSPlayer, Padding0 ) == 0x114 );
	C_ASSERT( sizeof( CNWSPlayer ) == 0x118 );

}


//
// Compatibility with NWN2GameLib.
//

typedef NWN::CNWSCreature CreatureObject;
typedef NWN::CNWSArea AreaObject;
typedef NWN::CGameObject GameObject;

//
// Define a (roughly) NWN2GameLib-compatible object management system.
//

class GameObjectManager
{

public:

	//
	// Return a game object by object id.  Returns NULL if there was no match.
	//

	inline
	GameObject *
	GetGameObject(
		__in NWN::OBJECTID ObjectId
		)
	{
		typedef
		NWN::CGameObject *
		(__cdecl * NWServer_GetGameObjectProc)(
			__in NWN::OBJECTID ObjectId
			);

		static NWServer_GetGameObjectProc NWServer_GetGameObject = (NWServer_GetGameObjectProc) OFFS_NWServerGetGameObject;

		return NWServer_GetGameObject( ObjectId );
	}

	//
	// Return a creature object by object id.  Returns NULL if there was no
	// such object, or if there was an object type mismatch.
	//

	inline
	CreatureObject *
	GetCreature(
		__in NWN::OBJECTID ObjectId
		)
	{
		GameObject * Object;

		if ((Object = GetGameObject( ObjectId )) == NULL)
			return NULL;

		return Object->AsCreature( );
	}

	//
	// Return an area object by object id.  Returns NULL if there was no
	// such object, or if there was an object type mismatch.
	//

	inline
	AreaObject *
	GetArea(
		__in NWN::OBJECTID ObjectId
		)
	{
		GameObject * Object;

		if ((Object = GetGameObject( ObjectId )) == NULL)
			return NULL;

		return Object->AsArea( );
	}

};


//
// Pull in common protocol definitions.
//

#include "NWN2ProtocolCommon.h"

//
// Pull in client to server protocol definitions
//

//#include "NWN2ProtocolClientToServer.h"

//
// Pull in server to client protocol definitions.
//

//#include "NWN2ProtocolServerToClient.h"



#endif
