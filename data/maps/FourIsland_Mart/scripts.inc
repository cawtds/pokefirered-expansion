FourIsland_Mart_MapScripts::
	.byte 0

FourIsland_Mart_EventScript_Clerk::
	goto_if_questlog EventScript_ReleaseEnd
	lock
	faceplayer
	message Text_MayIHelpYou
	waitmessage
	pokemart SHOP_ID_FOUR_ISLAND_MART
	msgbox Text_PleaseComeAgain
	release
	end

FourIsland_Mart_EventScript_Camper::
	msgbox FourIsland_Mart_Text_IcefallCaveIsFrigid, MSGBOX_NPC
	end
