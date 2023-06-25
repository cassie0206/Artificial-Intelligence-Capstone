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

class GameControl():
    """
    Minesweeper game representation
    GAME CONTROL
    """

    def __init__(self, height=HEIGHT, width=WIDTH, mines=MINES):
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

    def hint(self, cell):
        """
        Provide the hint (number of surrounding mines)
        """
        # for keep the number of surrounding mines
        count = 0
        for i in range(cell[0] - 1, cell[0] + 2):
            # if the position is illegal, then skip
            if i < 0 or i >= self.height:
                continue
            for j in range(cell[1] - 1, cell[1] + 2):
                # if the position is illegal or just the same with cell, then skip
                if j < 0 or j >= self.width or (i == cell[0] and j == cell[1]):
                    continue
                
                # the position has mine, so count++
                if self.board[i][j]:
                    count += 1

        return count
    
    def get_initial_safes(self):
        """
        get the initial safe list
        """
        return self.initial_safes
    
    def print_current(self, marked, flags):
        """
        Prints a text-based representation
        of where current board state.
        """
        for i in range(HEIGHT):
            print("--" * WIDTH + "-")
            for j in range(WIDTH):
                if (i, j) in flags:
                    print("|F", end="")
                elif (i, j) in marked:
                    text = "|%s" % str(self.hint((i, j)))
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
        # keeping the coordinate of the cells
        self.cells = set(cells)
        # keep the number of the surrouding mines
        self.count = count

    def __eq__(self, other):
        return self.cells == other.cells and self.count == other.count

    def __str__(self):
        return f"{self.cells} = {self.count}"

    def known_mines(self):
        """
        Returns the inferred mines
        """    
        res = set()
        # when the number of surrounding mins is equal to the surrounding unknown mines,
        # it means that all of them are mines 
        if len(self.cells) == self.count and self.count != 0:
            res = self.cells
        
        return res

    def known_safes(self):
        """
        Returns the inferred safe cells
        """
        res = set()
        # when the surrounding mines is 0,
        # it means that all the surrouding cells are safe
        if self.count == 0:
            res = self.cells
        
        return res

    def mark_mine(self, cell):
        """
        update the information to the sentence that there is a mine for sure
        """
        # if the mine is in the sentence,
        # then move it out and reduce the count by one
        if cell in self.cells:
            self.cells.remove(cell)
            self.count -= 1

    def mark_safe(self, cell):
        """
        update the information to the sentence that there is a safe cell for sure
        """
        # if the mine is in the sentence,
        # then move it out and no need to reduce the count
        # since it is count for the number of mines
        if cell in self.cells:
            self.cells.remove(cell)
            

class Player():
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
            self.KB.append(Sentence(cells, 0))

        # represent marked cell
        # set(cell) -> set(set())
        # TODO change to list()
        self.KB0 = set()

    def mark_mine(self, cell):
        """
        UNIT PROPAGATION for mines
        """
        self.mines.add(cell)
        for sentence in self.KB:
            sentence.mark_mine(cell)

    def mark_safe(self, cell):
        """
        UNIT PROPAGATION for safe cell
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
            1. generating new sentencce from the hints
            2. insert the clauses regarding its unmarked neighbors into the KB
        """
        unmarked_neibors = set()

        # 1. generating new sentencce from the hints
        for i in range(cell[0] - 1, cell[0] + 2):
            if i < 0 or i >= self.height:
                continue
            for j in range(cell[1] - 1, cell[1] + 2):
                if j < 0 or j >= self.width or (i == cell[0] and j == cell[1]):
                    continue

                # skip if marked
                if (i, j) in self.safes or (i, j) in self.mines or (i, j) in self.KB0:
                    if (i, j) in self.mines:
                        count -= 1
                    continue

                unmarked_neibors.add((i, j))

        new_sentence = Sentence(unmarked_neibors, count)
        safes = new_sentence.known_safes()
        mines = new_sentence.known_mines()
        
        # do resolution and insert the new sentence in KB
        if safes != set():
            keep = safes.copy()
            for safe in keep:
                self.mark_safe(safe)

        elif mines != set():
            keep = mines.copy()
            for mine in keep:
                self.mark_mine(mine)
                
        # Skip the insertion if there is an identical clause in KB.
        elif new_sentence in self.KB:
            pass

        else:
            self.KB.append(new_sentence)

    def matching(self):
        """
            1. duplication
            2. matching
            If new clauses are generated due to resolution, insert them into the KB or safes or mins
        """
        # 1. duplication
        # keep the sentence needed to be removed owning to duplication and resolution
        removed_sentence = []
        # keep the inferred sentence needed to be appended owning to resolution
        stricker_sentence = []

        if len(self.KB) != 0:
            for i in range(len(self.KB)):
                for j in range(len(self.KB)):
                    if i >= j: 
                        continue

                    sentence1 = self.KB[i]
                    sentence2 = self.KB[j]

                    # duplication
                    if sentence1 == sentence2 and sentence1.cells != set():
                        removed_sentence.append(sentence2)

                    # resolution
                    elif sentence1.cells.issubset(sentence2.cells):
                        stricker_sentence.append(Sentence(sentence2.cells - sentence1.cells, sentence2.count - sentence1.count))
                        removed_sentence.append(sentence2)

                    elif sentence1.cells.issuperset(sentence2.cells):
                        stricker_sentence.append(Sentence(sentence1.cells - sentence2.cells, sentence1.count - sentence2.count))
                        removed_sentence.append(sentence1)

        if len(removed_sentence) != 0:
            for sentence in removed_sentence:
                if sentence in self.KB:
                    self.KB.remove(sentence)
        
        if len(stricker_sentence) != 0:
            for sentence in stricker_sentence:
                if sentence not in self.KB:
                    self.KB.append(sentence)

        # 2. matching
        for sentence in self.KB:
            # check for safe cell
            safes = sentence.known_safes()
            if safes != set():
                keep = safes.copy()
                for safe in keep:
                    self.mark_safe(safe)

            mines = sentence.known_mines()
            if mines != set():
                keep = mines.copy()
                for mine in keep:
                    self.mark_mine(mine)


if __name__ == '__main__':
    parser = get_parser()
    turn = parser.turn
    toPrint = parser.show
    win = 0

    # ask the player for the game level
    ans = input('Please chose the difficulty:\n(A) Easy (9x9 board with 10 mines)\n\
(B) Medium (16x16 board with 25 mines)\n\
(C) Hard (30x16 board with 99 mines)\n')

    if ans == 'B' or ans == '(B)' or ans == 'b':
        HEIGHT, WIDTH, MINES = 16, 16, 25
    elif ans == 'C' or ans == '(C)' or ans == 'c':
        HEIGHT, WIDTH, MINES = 16, 30, 99

    for i in range(turn):
        # Create game and AI agent
        game = GameControl(height=HEIGHT, width=WIDTH, mines=MINES)
        ai = Player(game, height=HEIGHT, width=WIDTH)

        # Keep track of revealed cells, flagged cells, and if a mine was hit
        marked = set()
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
            elif len(flags) + len(marked) == HEIGHT * WIDTH:
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
                # Process the "matching" of that clause to all the remaining clauses in the KB.
                ai.matching()
                stuck = 0
                if move in ai.mines:
                    flags.add(move)
                elif game.is_mine(move):
                    lost = True
                else:
                    # this cell is safe
                    # Query the game control module for the hint at that cell
                    hint = game.hint(move)
                    ai.insert_new_clause(move, hint)
                    # keep for visulaizaton
                    marked.add(move)
            else:
                ai.matching()
                stuck += 1

            # print current board state
            if toPrint:
                game.print_current(marked, flags)

        # for checking use
        if toPrint:
            game.print()

    win /= turn
    print('\nWIN RATE: %f' % win)


