#pragma once

enum InputType {
	keyboard,
	mouseButton,
	gamepad1,
	gamepad2,
	gamepad3,
	gamepad4,
};

struct Button{
	InputType inputType;
	int key;

	Button() = default;
	constexpr Button(InputType inputType_, const int key_) : inputType(inputType_), key(key_) {}
	
	auto operator==(const Button& rhs) const { return inputType == rhs.inputType && key == rhs.key; };
	auto operator!=(const Button& rhs) const { return !(*this == rhs); }



	template <typename T>
	Button& operator+= (const T& y) {
		this->key += y;
		return *this;
	};
	template <typename T>
	Button& operator-= (const T& y) {
		this->key -= y;
		return *this;
	};

	template <typename T>
	Button operator+ (const T& y) const{
		Button temp = *this;
		temp += y;
		return temp;
	}

	template <typename T>
	Button operator- (const T& y) const {
		Button temp = *this;
		temp -= y;
		return temp;
	}
};

/* Printable keys */
constexpr Button KEY_SPACE =             {keyboard,  32};
constexpr Button KEY_APOSTROPHE =        {keyboard,  39};  /* ' */
constexpr Button KEY_COMMA =             {keyboard,  44};  /* , */
constexpr Button KEY_MINUS =             {keyboard,  45};  /* - */
constexpr Button KEY_PERIOD =            {keyboard,  46};  /* . */
constexpr Button KEY_SLASH =             {keyboard,  47};  /* / */
constexpr Button KEY_0 =                 {keyboard,  48};
constexpr Button KEY_1 =                 {keyboard,  49};
constexpr Button KEY_2 =                 {keyboard,  50};
constexpr Button KEY_3 =                 {keyboard,  51};
constexpr Button KEY_4 =                 {keyboard,  52};
constexpr Button KEY_5 =                 {keyboard,  53};
constexpr Button KEY_6 =                 {keyboard,  54};
constexpr Button KEY_7 =                 {keyboard,  55};
constexpr Button KEY_8 =                 {keyboard,  56};
constexpr Button KEY_9 =                 {keyboard,  57};
constexpr Button KEY_SEMICOLON =         {keyboard,  59};  /* ; */
constexpr Button KEY_EQUAL =             {keyboard,  61};  /* = */
constexpr Button KEY_A =                 {keyboard,  65};
constexpr Button KEY_B =                 {keyboard,  66};
constexpr Button KEY_C =                 {keyboard,  67};
constexpr Button KEY_D =                 {keyboard,  68};
constexpr Button KEY_E =                 {keyboard,  69};
constexpr Button KEY_F =                 {keyboard,  70};
constexpr Button KEY_G =                 {keyboard,  71};
constexpr Button KEY_H =                 {keyboard,  72};
constexpr Button KEY_I =                 {keyboard,  73};
constexpr Button KEY_J =                 {keyboard,  74};
constexpr Button KEY_K =                 {keyboard,  75};
constexpr Button KEY_L =                 {keyboard,  76};
constexpr Button KEY_M =                 {keyboard,  77};
constexpr Button KEY_N =                 {keyboard,  78};
constexpr Button KEY_O =                 {keyboard,  79};
constexpr Button KEY_P =                 {keyboard,  80};
constexpr Button KEY_Q =                 {keyboard,  81};
constexpr Button KEY_R =                 {keyboard,  82};
constexpr Button KEY_S =                 {keyboard,  83};
constexpr Button KEY_T =                 {keyboard,  84};
constexpr Button KEY_U =                 {keyboard,  85};
constexpr Button KEY_V =                 {keyboard,  86};
constexpr Button KEY_W =                 {keyboard,  87};
constexpr Button KEY_X =                 {keyboard,  88};
constexpr Button KEY_Y =                 {keyboard,  89};
constexpr Button KEY_Z =                 {keyboard,  90};
constexpr Button KEY_LEFT_BRACKET =      {keyboard,  91};  /* [ */
constexpr Button KEY_BACKSLASH =         {keyboard,  92};  /* \ */
constexpr Button KEY_RIGHT_BRACKET =     {keyboard,  93};  /* ] */
constexpr Button KEY_GRAVE_ACCENT =      {keyboard,  96};  /* ` */
constexpr Button KEY_WORLD_1 =           {keyboard,  161}; /* non-US #1 */
constexpr Button KEY_WORLD_2 =           {keyboard,  162}; /* non-US #2 */

/* Function keys */
constexpr Button KEY_ESCAPE =            {keyboard,  256};
constexpr Button KEY_ENTER =             {keyboard,  257};
constexpr Button KEY_TAB =               {keyboard,  258};
constexpr Button KEY_BACKSPACE =         {keyboard,  259};
constexpr Button KEY_INSERT =            {keyboard,  260};
constexpr Button KEY_DELETE =            {keyboard,  261};
constexpr Button KEY_RIGHT =             {keyboard,  262};
constexpr Button KEY_LEFT =              {keyboard,  263};
constexpr Button KEY_DOWN =              {keyboard,  264};
constexpr Button KEY_UP =                {keyboard,  265};
constexpr Button KEY_PAGE_UP =           {keyboard,  266};
constexpr Button KEY_PAGE_DOWN =         {keyboard,  267};
constexpr Button KEY_HOME =              {keyboard,  268};
constexpr Button KEY_END =               {keyboard,  269};
constexpr Button KEY_CAPS_LOCK =         {keyboard,  280};
constexpr Button KEY_SCROLL_LOCK =       {keyboard,  281};
constexpr Button KEY_NUM_LOCK =          {keyboard,  282};
constexpr Button KEY_PRINT_SCREEN =      {keyboard,  283};
constexpr Button KEY_PAUSE =             {keyboard,  284};
constexpr Button KEY_F1 =                {keyboard,  290};
constexpr Button KEY_F2 =                {keyboard,  291};
constexpr Button KEY_F3 =                {keyboard,  292};
constexpr Button KEY_F4 =                {keyboard,  293};
constexpr Button KEY_F5 =                {keyboard,  294};
constexpr Button KEY_F6 =                {keyboard,  295};
constexpr Button KEY_F7 =                {keyboard,  296};
constexpr Button KEY_F8 =                {keyboard,  297};
constexpr Button KEY_F9 =                {keyboard,  298};
constexpr Button KEY_F10 =               {keyboard,  299};
constexpr Button KEY_F11 =               {keyboard,  300};
constexpr Button KEY_F12 =               {keyboard,  301};
constexpr Button KEY_F13 =               {keyboard,  302};
constexpr Button KEY_F14 =               {keyboard,  303};
constexpr Button KEY_F15 =               {keyboard,  304};
constexpr Button KEY_F16 =               {keyboard,  305};
constexpr Button KEY_F17 =               {keyboard,  306};
constexpr Button KEY_F18 =               {keyboard,  307};
constexpr Button KEY_F19 =               {keyboard,  308};
constexpr Button KEY_F20 =               {keyboard,  309};
constexpr Button KEY_F21 =               {keyboard,  310};
constexpr Button KEY_F22 =               {keyboard,  311};
constexpr Button KEY_F23 =               {keyboard,  312};
constexpr Button KEY_F24 =               {keyboard,  313};
constexpr Button KEY_F25 =               {keyboard,  314};
constexpr Button KEY_KP_0 =              {keyboard,  320};
constexpr Button KEY_KP_1 =              {keyboard,  321};
constexpr Button KEY_KP_2 =              {keyboard,  322};
constexpr Button KEY_KP_3 =              {keyboard,  323};
constexpr Button KEY_KP_4 =              {keyboard,  324};
constexpr Button KEY_KP_5 =              {keyboard,  325};
constexpr Button KEY_KP_6 =              {keyboard,  326};
constexpr Button KEY_KP_7 =              {keyboard,  327};
constexpr Button KEY_KP_8 =              {keyboard,  328};
constexpr Button KEY_KP_9 =              {keyboard,  329};
constexpr Button KEY_KP_DECIMAL =        {keyboard,  330};
constexpr Button KEY_KP_DIVIDE =         {keyboard,  331};
constexpr Button KEY_KP_MULTIPLY =       {keyboard,  332};
constexpr Button KEY_KP_SUBTRACT =       {keyboard,  333};
constexpr Button KEY_KP_ADD =            {keyboard,  334};
constexpr Button KEY_KP_ENTER =          {keyboard,  335};
constexpr Button KEY_KP_EQUAL =          {keyboard,  336};
constexpr Button KEY_LEFT_SHIFT =        {keyboard,  340};
constexpr Button KEY_LEFT_CONTROL =      {keyboard,  341};
constexpr Button KEY_LEFT_ALT =          {keyboard,  342};
constexpr Button KEY_LEFT_SUPER =        {keyboard,  343};
constexpr Button KEY_RIGHT_SHIFT =       {keyboard,  344};
constexpr Button KEY_RIGHT_CONTROL =     {keyboard,  345};
constexpr Button KEY_RIGHT_ALT =         {keyboard,  346};
constexpr Button KEY_RIGHT_SUPER =       {keyboard,  347};
constexpr Button KEY_MENU =              {keyboard,  348};

constexpr int LAST_KEY = KEY_MENU.key + 1;

// main mouse Buttons
constexpr Button MOUSE_BUTTON_LEFT   = { mouseButton, 0 };
constexpr Button MOUSE_BUTTON_RIGHT  = { mouseButton, 1 };
constexpr Button MOUSE_BUTTON_MIDDLE = { mouseButton, 2 };

constexpr int LAST_MOUSE_BUTTON = MOUSE_BUTTON_MIDDLE.key;

constexpr Button GAMEPAD1_A = { gamepad1, 0 };
constexpr Button GAMEPAD1_B = { gamepad1, 1 };
constexpr Button GAMEPAD1_X = { gamepad1, 2 };
constexpr Button GAMEPAD1_Y = { gamepad1, 3 };
constexpr Button GAMEPAD1_LB = { gamepad1, 4 };
constexpr Button GAMEPAD1_RB = { gamepad1, 5 };
constexpr Button GAMEPAD1_BACK = { gamepad1, 6 };
constexpr Button GAMEPAD1_START = { gamepad1, 7 };
constexpr Button GAMEPAD1_UP = { gamepad1, 11 };
constexpr Button GAMEPAD1_RIGHT = { gamepad1, 12 };
constexpr Button GAMEPAD1_DOWN = { gamepad1, 13 };
constexpr Button GAMEPAD1_LEFT = { gamepad1, 14 };

constexpr int LAST_GAMEPAD = GAMEPAD1_LEFT.key + 1;

// TODO: Implement gamepad.
// gamepad implementation must support multiple gamepads (likely through multiple inputType's for each device)