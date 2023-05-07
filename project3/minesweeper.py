import pygame
import sys
import time
import itertools
import random
import math
import argparse

# false -> safe
# true -> mine

# TODO: changing this can apply to different board size
# e.g. 9 * 9 -> 10 mines, 16 * 16 -> 25 mines, 30 * 16 -> 99 mines
HEIGHT = 9
WIDTH = 9
MINES = 10

def get_parser():
    parser = argparse.ArgumentParser(description="the number of turn")
    parser.add_argument("-t", "--turn", default=1, type=int)
    parser.add_argument("-s", "--show", default=0, type=int)
    
    return parser.parse_args()

class Minesweeper():
    """
    Minesweeper game representation
    GAME CONTROL
    """

    def __init__(self, height=HEIGHT, width=WIDTH, mines=MINES):
        # TODO: change board size
        # Set initial width, height, and number of mines
        self.height = height
        self.width = width
        self.mines = set()

        # Initialize an empty field with no mines
        self.board = []
        for i in range(self.height):
            row = []
            for j in range(self.width):
                row.append(False)
            self.board.append(row)

        # Add mines randomly
        while len(self.mines) != mines:
            i = random.randrange(height)
            j = random.randrange(width)
            if not self.board[i][j]:
                self.mines.add((i, j))
                self.board[i][j] = True

        # At first, player has found no mines
        self.mines_found = set()

        # initial safe list
        # TODO change the number of initial safe list to observer the win rate
        self.initial_safes = []
        while len(self.initial_safes) != int(round(math.sqrt(self.width * self.height), 0)):
            i = random.randrange(height)
            j = random.randrange(width)
            if not self.board[i][j]:
                self.initial_safes.append((i, j))

    def print(self):
        """
        Prints a text-based representation
        of where mines are located.
        """
        for i in range(self.height):
            print("--" * self.width + "-")
            for j in range(self.width):
                if self.board[i][j]:
                    print("|X", end="")
                else:
                    print("| ", end="")
            print("|")
        print("--" * self.width + "-")

    def is_mine(self, cell):
        i, j = cell
        return self.board[i][j]

    def nearby_mines(self, cell):
        """
        Returns the number of mines that are
        within one row and column of a given cell,
        not including the cell itself.
        """

        # Keep count of nearby mines
        count = 0

        # Loop over all cells within one row and column
        for i in range(cell[0] - 1, cell[0] + 2):
            for j in range(cell[1] - 1, cell[1] + 2):

                # Ignore the cell itself
                if (i, j) == cell:
                    continue

                # Update count if cell in bounds and is mine
                if 0 <= i < self.height and 0 <= j < self.width:
                    if self.board[i][j]:
                        count += 1

        return count

    def won(self):
        """
        Checks if all mines have been flagged.
        """
        return self.mines_found == self.mines
    
    def get_initial_safes(self):
        """
        get the initial safe list
        """
        return self.initial_safes
    
    def print_current(self, revealed, flags):
        """
        Prints a text-based representation
        of where current board state.
        """
        for i in range(HEIGHT):
            print("--" * WIDTH + "-")
            for j in range(WIDTH):
                if (i, j) in flags:
                    print("|F", end="")
                elif (i, j) in revealed:
                    text = "|%s" % str(self.nearby_mines((i, j)))
                    print(text, end="")
                else:
                    print("| ", end="")
            print("|")
        print("--" * WIDTH + "-")


class Sentence():
    """
    Logical statement about a Minesweeper game
    A sentence consists of a set of board cells,
    and a count of the number of those cells which are mines.
    """

    def __init__(self, cells, count):
        self.cells = set(cells)
        self.count = count

    def __eq__(self, other):
        return self.cells == other.cells and self.count == other.count

    def __str__(self):
        return f"{self.cells} = {self.count}"

    def known_mines(self):
        """
        Returns the set of all cells in self.cells known to be mines.
        """

        # Stores known mines
        mines = set()

        # Any time the number of cells is equal to the count (and count of mines is != 0), we know that all of that sentence's cells must be mines.
        if len(self.cells) == self.count and self.count != 0:
            mines = self.cells 
            return mines 
        else:
            return mines

    def known_safes(self):
        """
        Returns the set of all cells in self.cells known to be safe.
        """

        # Stores known safes
        safes = set()

        # Each time we have a sentence whose count is 0, we know that all the surrounding cells are safe
        if self.count == 0:
            safes = self.cells 
            return safes 
        else:
            return safes

    def mark_mine(self, cell):
        """
        Updates internal knowledge representation given the fact that
        a cell is known to be a mine.
        """
        # If a cell known to be a mine is in the sentence, remove it and decrement the sentence mine count by one as there is now one less mine in the sentence
        if cell in self.cells:
            self.cells.remove(cell)
            self.count -= 1
            return None
        
        # If cell is not in the sentence, then no action is necessary.

    def mark_safe(self, cell):
        """
        Updates internal knowledge representation given the fact that
        a cell is known to be safe.
        """
        # If a cell known to be safe is in the sentence, remove it without decrementing the mine count
        if cell in self.cells:
            self.cells.remove(cell)
            return None

class MinesweeperAI():
    """
    Minesweeper game player
    PLAYER
    """
    def __init__(self, game, height=HEIGHT, width=WIDTH):
        # TODO change size
        # Set initial height and width
        self.height = height
        self.width = width

        # Keep track of cells known to be safe or mines
        self.mines = set()
        # safes(cell) -> set(tuple) 
        self.safes = set()

        # List of sentences about the game known to be true
        # KB(sentence) -> list(set())
        self.KB = []
        # self.KB = game.get_initial_safes()
        # print(self.KB[0])
        for cell in game.get_initial_safes():
            self.safes.add(cell)
            cells = set()
            cells.add(cell)
            #self.KB.append(Sentence(cells, 0))

        # print('current KB:')
        #for sentence in self.KB:
        #    print(sentence.cells, end=' ')
        #    print(sentence.count)

        # Keep track of which cells have been clicked on
        # self.moves_made = set()
        # represent marked cell
        # set(cell) -> set(set())
        # TODO change to list()
        self.KB0 = set()

    def mark_mine(self, cell):
        """
        UNIT PROPAGATION
        Marks a cell as a mine, and updates all knowledge
        to mark that cell as a mine as well.
        """
        self.mines.add(cell)
        for sentence in self.KB:
            sentence.mark_mine(cell)

    def mark_safe(self, cell):
        """
        UNIT PROPAGATION
        Marks a cell as safe, and updates all knowledge
        to mark that cell as safe as well.
        """
        self.safes.add(cell)
        for sentence in self.KB:
            sentence.mark_safe(cell)

    def make_move(self, toPrint):
        """
        Mark that cell as safe or mine, move that clause to KB0.
        """
        # the returned move
        moves = set()
        if toPrint:
            print('current KB:')
            for sentence in self.KB:
                print(sentence.cells, end=' ')
                print(sentence.count)
        """
        for sentence in self.KB:
            if len(sentence.cells) == 1 and sentence.cells not in self.KB0:
                cell = sentence.cells.pop()
                if sentence.count == 0:
                    # Mark that cell as safe
                    self.mark_safe(cell)
                else:
                    # Mark that cell as mine
                    self.mark_mine(cell)
        """
        
        for safe in self.safes:
            if safe not in self.KB0:
                # Move that calsue to KB0
                self.KB0.add(safe)
                self.mark_safe(safe)
                moves.add(safe)
                break

        if len(moves) != 0:
            return moves.pop()
        else:
            for mine in self.mines:
                if mine not in self.KB0:
                    # Move that calsue to KB0
                    self.KB0.add(mine)
                    self.mark_mine(mine)
                    moves.add(mine)
                    break      
        
            if len(moves) == 0:
                return None
            else:
                return moves.pop()
    
    def insert_new_clause(self, cell, count):
        """
            insert the clauses regarding its unmarked neighbors into the KB
        """
        # Stores neighboring cells
        adjacent_cells = set()

        # Do resolution of the new clause with all the clauses in KB0
        for i in range(cell[0] - 1, cell[0] + 2):
            if i < 0 or i >= self.height:
                continue
            for j in range(cell[1] - 1, cell[1] + 2):
                if j < 0 or j >= self.width or (i == cell[0] and j == cell[1]):
                    continue

                if (i, j) in self.mines:
                    count -= 1
                    continue

                if (i, j) in self.safes or (i, j) in self.KB0:
                    continue
                
                adjacent_cells.add((i, j))

        # Creates a new sentence based on the cell's neighboring cells and the count 
        new_sentence = Sentence(adjacent_cells, count)

        # Check if new_sentence should be added to the knowledge base based on based on it's value of `cell` and `count`. Make a copy of the new_sentence_safe_cells to avoid modifying it while checking if it should be added to the knowledge base
        new_sentence_safe_cells = new_sentence.cells 
        new_sentence_safe_cells_copy = new_sentence_safe_cells.copy()
    
        # No need to add new_sentence with count = 0 to knowledge base as all cells are safe, mark them as such
        if new_sentence.count == 0:
            for cell in new_sentence_safe_cells_copy:
                self.mark_safe(cell)

        # if the number of mines is equal to the count of mine, which means all the cells are mine
        elif new_sentence.count == len(new_sentence.cells):
            for cell in new_sentence_safe_cells_copy:
                self.mark_mine(cell)

        else:
            self.KB.append(new_sentence)

    def matching(self):
        """
            4) mark any additional cells as safe or as mines
               if it can be concluded based on the AI's knowledge base
            5) subset & duplication
        """
        # 5) subset & duplication

        # Stores the inferred_sentences that will be added to the knowledge base
        inferred_knowledge = []

        # Compare cells and count for 2 sentences (s1 and s2) extracted from the knowledge base
        if self.KB is not []:
            for sentence1 in self.KB:
                for sentence2 in self.KB:
                # If there are 2 sentences in the knowledge_chunker
                    if sentence1 != sentence2:
                        # Create set1
                        set1 = sentence1
                        set1_cells = set1.cells 
                        set1_count = set1.count

                        # Create set2
                        set2 = sentence2
                        set2_cells = set2.cells 
                        set2_count = set2.count 

                        # Remove any duplicates 
                        if set1 == set2:
                            self.KB.remove(set2)

                        elif set1_cells != set2_cells:
                            # Check if set1 is a subset of set2
                            if set1_cells.issubset(set2_cells):
                                inferred_cells = set2_cells - set1_cells
                                inferred_count = set2_count - set1_count 
                                inferred_sentence = Sentence(inferred_cells, inferred_count)
                                # Add new sentence to inferred knowledge base
                                inferred_knowledge.append(inferred_sentence)

                            # Check if set2 is a subset of set1
                            elif set2_cells.issubset(set1_cells):
                                inferred_cells = set1_cells - set2_cells
                                inferred_count = set1_count - set2_count
                                inferred_sentence = Sentence(inferred_cells, inferred_count)
                                # Add inferred sentence to inferred knowledge base
                                inferred_knowledge.append(inferred_sentence)
        
        # If no inferred knowledge, don't modify the knowledge base
        if len(inferred_knowledge) != 0:
            # Add inferred sentence to the knowledge base if it isn't present yet
            for inferred in inferred_knowledge:
                if inferred not in self.KB:
                    self.KB.append(inferred)
                    
        # 4) If, based on any of the sentences in self.KB, new cells can be marked as safe or as mines, then the function should do so.
        # MATCHING
        for sentence in self.KB:
            # As the number of safes in the safe_cells variable changes with each iteration (as I am marking safes), create a copy of the initial set against which to mark safes 
            safe_cells = sentence.known_safes()
            safe_cells_copy = safe_cells.copy()

            if safe_cells:
                for cell in safe_cells_copy:
                    self.mark_safe(cell)
            
            # As the number of mines in the mine_cells variable changes with each iteration (as I am marking mines), create a copy of the initial set against which to mark mines 
            mine_cells = sentence.known_mines()
            mine_cells_copy = mine_cells.copy()

            if mine_cells:
                for cell in mine_cells_copy:
                    self.mark_mine(cell)



if __name__ == '__main__':
    parser = get_parser()
    turn = parser.turn
    toPrint = parser.show
    win = 0

    ans = input('Please chose the difficulty:\n(A) Easy (9x9 board with 10 mines)\n\
(B) Medium (16x16 board with 25 mines)\n\
(C) Hard (30x16 board with 99 mines)\n')

    if ans == 'B' or ans == '(B)' or ans == 'b':
        HEIGHT, WIDTH, MINES = 16, 16, 25
    elif ans == 'C' or ans == '(C)' or ans == 'c':
        HEIGHT, WIDTH, MINES = 30, 16, 99

    for i in range(turn):
        # Create game and AI agent
        game = Minesweeper(height=HEIGHT, width=WIDTH, mines=MINES)
        ai = MinesweeperAI(game, height=HEIGHT, width=WIDTH)

        # Keep track of revealed cells, flagged cells, and if a mine was hit
        revealed = set()
        flags = set()
        lost = False

        # Show instructions initially
        instructions = True
        initial = True
        count = 1
        stuck = 0

        while 1:
            # show welcome 
            if instructions and toPrint:
                print('********************************************WELCOME TO MINESWEEPER********************************************')
                print('self initial safe list:')
                for safe in game.initial_safes:
                    print(safe, end=' ')
                print('mine:')
                print(game.mines)
                initial = False
                game.print()
                instructions = False
            
            # win or lose or not yet
            if lost:
                print('Game #%s LOSE' % str(i + 1))
                break
            elif flags == game.mines:
                print('Game #%s WIN' % str(i + 1))
                win += 1
                break
            elif stuck == 5:
                print('Game #%s STUCK' % str(i + 1))
                print(len(flags))
                break
            elif toPrint:
                print('Turn #%d' % count)
                count += 1

            # start game flow
            move = ai.make_move(toPrint)
            # move = ai.make_safe_move()
            if toPrint:
                print('move:', end=' ')
                print(move)
            
            if move:
                if move in ai.mines:
                    flags.add(move)
                elif game.is_mine(move):
                    lost = True
                else:
                    # this cell is safe
                    # Query the game control module for the hint at that cell
                    ai.matching()
                    nearby = game.nearby_mines(move)
                    ai.insert_new_clause(move, nearby)
                    # ai.matching()
                    revealed.add(move)
            else:
                ai.matching()
                stuck += 1

            # print current board state
            if toPrint:
                game.print_current(revealed, flags)

        # for checking use
        if toPrint:
            game.print()

    win /= turn
    print('\nWIN RATE: %f' % win)


