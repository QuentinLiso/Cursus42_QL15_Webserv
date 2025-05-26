/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   3_Config.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/22 08:45:05 by qliso             #+#    #+#             */
/*   Updated: 2025/05/26 11:39:38 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_DATA_STRUCTURES_HPP
# define CONFIG_DATA_STRUCTURES_HPP

# include "Includes.hpp"
# include "0_Utils.hpp"
# include "0A_DataStructures.hpp"
# include "1_Lexing.hpp"
# include "2_Parsing.hpp"



class	Builder
{
	private:
		std::vector<AConfigBlock*>	_build;

		void	nodeSemanticAnalysis(AParsingNode* node);
		void	astSemanticAnalysis(const std::vector<AParsingNode*>& ast);
		

	public:
		Builder(const std::vector<AParsingNode*>& ast);
		~Builder(void);
};


#endif