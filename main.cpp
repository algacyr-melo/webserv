/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: almelo <almelo@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/05 15:31:18 by almelo            #+#    #+#             */
/*   Updated: 2024/02/06 00:56:12 by almelo           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "http_TcpServer.hpp"

int main(void)
{
	using namespace http;

	int const PORT = 8080;

	TcpServer server = TcpServer("0.0.0.0", PORT);

	server.startListen();
	return 0;
}
