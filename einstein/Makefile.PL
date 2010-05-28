
use ExtUtils::MakeMaker;

WriteMakefile(
	NAME	=> 'Game::Einstein',
	VERSION_FROM => 'Einstein.pm',
	OBJECT	=> 'Einstein$(OBJ_EXT) game$(OBJ_EXT) try$(OBJ_EXT) rule$(OBJ_EXT)',
);
