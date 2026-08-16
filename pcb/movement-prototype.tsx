import { Microcontroller_RP2040 } from "@tscircuit/common"

const rowNets = ["net.ROW0", "net.ROW1", "net.ROW2"] as const
const colNets = ["net.COL0", "net.COL1", "net.COL2", "net.COL3"] as const

const keyDefinitions = [
  { id: 1, row: 0, col: 0, label: "MOVEMENT" },
  { id: 2, row: 0, col: 1, label: "TYPE" },
  { id: 3, row: 0, col: 2, label: "PREV LINE" },
  { id: 4, row: 0, col: 3, label: "NEXT LINE" },
  { id: 5, row: 1, col: 0, label: "START LINE" },
  { id: 6, row: 1, col: 1, label: "END LINE" },
  { id: 7, row: 1, col: 2, label: "NEXT WORD" },
  { id: 8, row: 1, col: 3, label: "NEXT SPACE WORD" },
  { id: 9, row: 2, col: 0, label: "7 TIMES" },
  { id: 10, row: 2, col: 1, label: "13 TIMES" },
  { id: 11, row: 2, col: 2, label: "DELETE CHAR" },
  { id: 12, row: 2, col: 3, label: "DELETE WORD" },
] as const

const pitch = 19.05
const matrixCenterX = 20
const matrixCenterY = 0

const HotSwapKey = ({
  id,
  row,
  col,
  label,
}: (typeof keyDefinitions)[number]) => {
  const switchName = `K${id}`
  const diodeName = `D${id}`
  const pcbX = matrixCenterX + (col - 1.5) * pitch
  const pcbY = matrixCenterY + (1 - row) * pitch

  return (
    <group
      pcbX={pcbX}
      pcbY={pcbY}
      schX={col * 4}
      schY={(1 - row) * 3}
    >
      <pushbutton
        name={switchName}
        layer="bottom"
        footprint={
          <footprint>
            <hole pcbX="3.175mm" pcbY="-1.27mm" diameter="3mm" />
            <hole pcbX="-3.175mm" pcbY="1.27mm" diameter="3mm" />
            <smtpad
              portHints={["pin2"]}
              pcbX="6.725mm"
              pcbY="-1.27mm"
              width="2.9mm"
              height="2.5mm"
              shape="rect"
            />
            <smtpad
              portHints={["pin1"]}
              pcbX="-6.725mm"
              pcbY="1.27mm"
              width="2.9mm"
              height="2.5mm"
              shape="rect"
            />
            <silkscreenrect
              pcbX="0.635mm"
              pcbY="-3.115mm"
              width="15.6mm"
              height="15.6mm"
            />
          </footprint>
        }
      />
      <hole
        name={`${switchName}_SHAFT`}
        pcbX="0.635mm"
        pcbY="-3.115mm"
        diameter="4.2mm"
      />
      <diode
        name={diodeName}
        footprint="sod123"
        layer="bottom"
        pcbX={0}
        pcbY={6.5}
        pcbRotation={0}
      />
      <trace from={`.${switchName} > .pin1`} to={`.${diodeName} > .anode`} />
      <trace from={`.${diodeName} > .cathode`} to={rowNets[row]} />
      <trace from={`.${switchName} > .pin2`} to={colNets[col]} />
      <silkscreentext text={label} pcbY={-8.5} />
    </group>
  )
}

export default function MovementPrototype() {
  return (
    <board
      name="MOVEMENT_PROTOTYPE"
      width="132mm"
      height="70mm"
      autorouter="auto_local"
      autorouterEffortLevel="10x"
    >
      <net name="ROW0" />
      <net name="ROW1" />
      <net name="ROW2" />
      <net name="COL0" />
      <net name="COL1" />
      <net name="COL2" />
      <net name="COL3" />

      <Microcontroller_RP2040
        name="MCU"
        pcbX={-49}
        pcbY={0}
        schX={-12}
        schY={0}
        connections={{
          GPIO0: rowNets[0],
          GPIO1: rowNets[1],
          GPIO2: rowNets[2],
          GPIO3: colNets[0],
          GPIO4: colNets[1],
          GPIO5: colNets[2],
          GPIO6: colNets[3],
        }}
      />

      {keyDefinitions.map((definition) => (
        <HotSwapKey key={definition.id} {...definition} />
      ))}

      <hole name="MOUNT_NW" diameter="3.2mm" pcbX={-62} pcbY={31} />
      <hole name="MOUNT_SW" diameter="3.2mm" pcbX={-62} pcbY={-31} />
      <hole name="MOUNT_NE" diameter="3.2mm" pcbX={62} pcbY={31} />
      <hole name="MOUNT_SE" diameter="3.2mm" pcbX={62} pcbY={-31} />

      <silkscreentext text="MOVEMENT KEYPAD PROTOTYPE" pcbX={7} pcbY={32} />
      <silkscreentext text="USB-C / RP2040 / 3x4 MATRIX" pcbX={7} pcbY={-32} />
    </board>
  )
}
