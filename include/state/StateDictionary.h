//
// Created by Paul Tortel on 20/05/2019.
//

#ifndef PATHFINDING_PROJECT_STATEDICTIONARY_H
#define PATHFINDING_PROJECT_STATEDICTIONARY_H

#include <ostream>
#include <iostream>
#include "State.h"

struct StateDictionary {
    /**
     * Time_step (starting from 0 with the initial state) => State
     */
    std::map<int, State> dictionary;

    /**
     * Add or update the position (in the proper state) of an agent at a given time step
     * @param init_state : the initial state to initialize a new state if necessary
     * @param time_step : the current time step
     * @param agent_id : the agent id that we want to change/update its position
     * @param new_search_square : the new position of the agent in the state
     */
    void addOrUpdateState(const State &init_state, const int time_step, const int agent_id,
                          const std::shared_ptr<SearchSquare> &new_search_square) {
        if (dictionary.find(time_step) != dictionary.end()) {
            // Not new
            dictionary[time_step].setSearchSquareForAgent(agent_id, new_search_square);

        } else {
            // New
            State new_state;

            if (dictionary.empty()) {
                new_state = init_state;
            } else {
                new_state = dictionary.rbegin()->second;
            }

            new_state.setSearchSquareForAgent(agent_id, new_search_square);
            dictionary[time_step] = new_state;
        }
    }

    /**
     * Sets the position of agent at a given time step
     * @param time_step : the given time step
     * @param agent_id : the agent ID
     * @param search_square : the new search square
     */
    void setAgentPositionFromTimeStep(int time_step, const int& agent_id, const std::shared_ptr<SearchSquare>& search_square) {

        std::shared_ptr<SearchSquare> next_search_square = search_square;

        while (dictionary.find(time_step) != dictionary.end()) {

            dictionary[time_step].setSearchSquareForAgent(agent_id, next_search_square);
            time_step++;

            //TODO : to analyze /// TO REMOVE?
            /*next_search_square = std::make_shared<SearchSquare>
                    (next_search_square->position, next_search_square, next_search_square->cost_movement, next_search_square->cost_heuristic);*/
        }
    }

    /**
     * Returns a pointer of the state, found from a given time step
     * @param time_step : the time step
     * @return a pointer of the state found, nullptr if nothing is found
     */
    State * getStateFromTimeStep(const int &time_step) {

        if (dictionary.find(time_step) != dictionary.end()) {
            return &dictionary[time_step];
        }

        return dictionary.empty() ? nullptr : &dictionary.rbegin()->second;
    }

    friend std::ostream &operator<<(std::ostream &os, const StateDictionary &_dictionary) {
        os << "Sol: " << std::endl;
        for (auto& it : _dictionary.dictionary) {
            os << "T:" << it.first << std::endl << it.second << std::endl;
        }
        os << std::endl;
        return os;
    }

    /**
     * Detects an edge conflict between two successive states and returns the first one we find
     * @param time_step : the given time step
     * @param it_current_state : the current state
     * @param it_next_state : the next state
     * @return
     */
    std::unique_ptr<EdgeConflict> detectFirstEdgeConflictFromTwoStates(const int& time_step, std::map<int, State>::iterator it_current_state,
            std::map<int, State>::iterator it_next_state) {

        for (auto& it_agent : it_current_state->second.getSearchSquares()) {
            Position& curr_pos = it_agent.second->position;
            Position& next_pos = it_next_state->second.getSearchSquares().at(it_agent.first)->position;

            std::unique_ptr<EdgeConflict> edge_conflict = getEdgeConflictWithOtherAgents(curr_pos, next_pos, extractDirection(curr_pos, next_pos),
                    time_step, &it_current_state->second, it_agent.first);

            if (edge_conflict != nullptr) {
                return edge_conflict;
            }
        }
        return nullptr;
    }

    /**
     * Return an edge conflict if the agent will collide with other agents if it moves in a given direction
     * @param current_position : the current agent position
     * @param next_position : the next agent position that we are evaluating
     * @param direction : the direction such movement would do
     * @param time_step : the current time step
     * @param current_state : a pointer to the current state
     * @param agent_id : the id of the agent
     * @return a unique pointer on an edge conflict, nullptr if no collision has been detected
     */
    std::unique_ptr<EdgeConflict>
    getEdgeConflictWithOtherAgents(const Position &current_position, Position &next_position,
                                   const Direction &direction,
                                   const int &time_step, State *current_state, const int &agent_id) {

        if (current_state == nullptr) {
            return nullptr;
        }

        Position pos_right = Position(current_position.x+1, current_position.y);
        Position pos_up = Position(current_position.x, current_position.y+1);
        Position pos_left = Position(current_position.x-1, current_position.y);
        Position pos_down = Position(current_position.x, current_position.y-1);
        Position pos_down_left = Position(current_position.x-1, current_position.y-1);
        Position pos_down_right = Position(current_position.x+1, current_position.y-1);
        Position pos_up_left = Position(current_position.x-1, current_position.y+1);
        Position pos_up_right = Position(current_position.x+1, current_position.y+1);

        std::unique_ptr<EdgeConflict> edge_conflict = nullptr;

        switch(direction) {

            case NO_DIRECTION:break;

            case NORTH:
                edge_conflict = detectEdgeCollisionWithNeighbour(current_position, next_position, time_step,
                                                                 *current_state, pos_up, agent_id);
                if (edge_conflict != nullptr) {
                    return edge_conflict;
                }
                break;
            case SOUTH:
                edge_conflict = detectEdgeCollisionWithNeighbour(current_position, next_position, time_step,
                                                                 *current_state, pos_down, agent_id);
                if (edge_conflict != nullptr) {
                    return edge_conflict;
                }
                break;
            case EAST:
                edge_conflict = detectEdgeCollisionWithNeighbour(current_position, next_position, time_step,
                                                                 *current_state, pos_right, agent_id);
                if (edge_conflict != nullptr) {
                    return edge_conflict;
                }
                break;
            case WEST:
                edge_conflict = detectEdgeCollisionWithNeighbour(current_position, next_position, time_step,
                                                                 *current_state, pos_left, agent_id);
                if (edge_conflict != nullptr) {
                    return edge_conflict;
                }
                break;
            case NE:break;
            case NW:break;
            case SE:break;
            case SW:break;
        }

        return nullptr;
    }

    /**
     * Return an unique pointer on the edge conflict detect
     * @param current_position : the current agent position
     * @param next_position : the next agent position that we are evaluating
     * @param time_step : the current time step
     * @param current_state : the current state
     * @param current_position_neighbour : the current position of the neighbour agent
     * @param agent_id : the id of the agent
     * @return an unique pointer on the edge conflict detect
     */
    std::unique_ptr<EdgeConflict>
    detectEdgeCollisionWithNeighbour(const Position &current_position, const Position &next_position,
                                     const int &time_step,
                                     State &current_state, const Position &current_position_neighbour,
                                     const int &agent_id) {

        // We check if there is an agent at the position that could lead to a collision with our agent
        // if this agent moves in a particular direction
        auto &it_agent = current_state.findAgentAtPosition(current_position_neighbour);

        if (it_agent != current_state.getSearchSquares().end()) {
            // We fetch the next state to be able to see where this agent will go at the next time step
            State* next_state = getStateFromTimeStep(time_step + 1);

            // There is no next state yet, then it means that the other agent will not move at this point
            if (next_state == nullptr) {
                // There is no collision
                return nullptr;
            }

            // If the movements will result in a collision
            if (areMovementsEdgeColliding(current_position, next_position, it_agent->second->position,
                                          next_state->getSearchSquares().at(it_agent->first)->position)) {
                return std::make_unique<EdgeConflict>(agent_id, it_agent->first, time_step+1,
                        current_position, it_agent->second->position,
                        next_position, next_state->getSearchSquares().at(it_agent->first)->position);
            }
        }
        // There is no agent
        return nullptr;
    }
};



#endif //PATHFINDING_PROJECT_STATEDICTIONARY_H
