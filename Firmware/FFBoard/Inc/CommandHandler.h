/*
 * CommandHandler.h
 *
 *  Created on: 03.04.2020
 *      Author: Yannick
 */

#ifndef COMMANDHANDLER_H_
#define COMMANDHANDLER_H_
#include <CmdParser.h>
#include "ClassChooser.h"

enum class ParseStatus : uint8_t {NOT_FOUND,OK,ERR,OK_CONTINUE};

/*
 * Implements an interface for parsed command handlers
 * Adds itself to a global vector of handlers that can be called from the main class when a command gets parsed
 * For example motor drivers and button sources can implement this to easily get serial commands
 */
class CommandHandler {
public:
	static std::vector<CommandHandler*> cmdHandlers;

	CommandHandler();
	virtual ~CommandHandler();
	virtual bool hasCommands();
	virtual void setCommandsEnabled(bool enable);
	virtual ParseStatus command(ParsedCommand* cmd,std::string* reply);
	virtual const ClassIdentifier getInfo() = 0; // Command handlers always have class infos. Works well with ChoosableClass
	virtual std::string getHelpstring(); // Returns a help string if "help" command is sent

protected:
	bool commandsEnabled = true;
	virtual void addCommandHandler();
	virtual void removeCommandHandler();

	template<typename TVal>
 	bool handleGetSet(ParsedCommand* cmd, std::string *reply, TVal& val) {
 		if (cmd->type == CMDtype::set) {
 			val = TVal(cmd->val);
			return true;
 		} else if (cmd->type == CMDtype::get) {
 			*reply += std::to_string(val);
 		}

		return false;
 	}

private:

};

#endif /* COMMANDHANDLER_H_ */
