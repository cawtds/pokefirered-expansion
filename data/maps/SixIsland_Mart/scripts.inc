SixIsland_Mart_MapScripts::
	.byte 0

SixIsland_Mart_EventScript_Clerk::
	goto_if_questlog EventScript_ReleaseEnd
	lock
	faceplayer
	message Text_MayIHelpYou
	waitmessage
	pokemart_by_id SHOP_ID_SIX_ISLAND_MART
	msgbox Text_PleaseComeAgain
	release
	end

SixIsland_Mart_EventScript_Picnicker::
	msgbox SixIsland_Mart_Text_ShouldBuyMailAndWriteLetter, MSGBOX_NPC
	end
