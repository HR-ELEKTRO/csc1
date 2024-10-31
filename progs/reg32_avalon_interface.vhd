-- altera vhdl_input_version vhdl_2008
library IEEE;
use IEEE.std_logic_1164.all;

entity reg32_avalon_interface is
	port (
		clock, resetn : in std_logic;
		read, write, chipselect : in std_logic;
		readdata : out std_logic_vector(31 downto 0);
		writedata : in std_logic_vector(31 downto 0);
		byteenable : in std_logic_vector(3 downto 0);
		Q_export : out std_logic_vector(31 downto 0)
	);
end reg32_avalon_interface;

architecture rtl of reg32_avalon_interface is
	type registers is array (0 to 0) of std_logic_vector(31 downto 0);
	signal regs: registers;
begin
	process(clock, resetn)
	begin
		if not resetn then
			for i in 0 to 0 loop
				regs(i) <= (others => '0');
			end loop;
		elsif rising_edge(clock) then
			if chipselect then
				if read then
					readdata <= regs(0);
				elsif write then
					if byteenable(0) then
						regs(0)(7 downto 0) <= writedata(7 downto 0);
					end if;
					if byteenable(1) then
						regs(0)(15 downto 8) <= writedata(15 downto 8);
					end if;
					if byteenable(2) then
						regs(0)(23 downto 16) <= writedata(23 downto 16);
					end if;
					if byteenable(3) then
						regs(0)(31 downto 24) <= writedata(31 downto 24);
					end if;
				end if;
			end if;
		end if;
	end process;
	Q_export <= regs(0);
end architecture rtl;