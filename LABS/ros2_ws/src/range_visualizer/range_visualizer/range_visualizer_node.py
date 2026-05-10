#!/usr/bin/env python3
"""Suscriptor a /ultrasonic/range: modo texto o rejilla mínima para demo."""

import math

import rclpy
from rclpy.node import Node
from nav_msgs.msg import MapMetaData, OccupancyGrid
from sensor_msgs.msg import Range


class RangeVisualizer(Node):
    def __init__(self) -> None:
        super().__init__('range_visualizer')
        self.declare_parameter('mode', 'text')
        self.declare_parameter('topic', '/ultrasonic/range')
        self.declare_parameter('grid_topic', '/ultrasonic/grid_demo')
        self.declare_parameter('grid_cols', 40)
        self.declare_parameter('resolution', 0.05)

        mode = self.get_parameter('mode').get_parameter_value().string_value
        topic = self.get_parameter('topic').get_parameter_value().string_value
        self._grid_topic = self.get_parameter('grid_topic').get_parameter_value().string_value
        self._grid_cols = self.get_parameter('grid_cols').get_parameter_value().integer_value
        self._resolution = self.get_parameter('resolution').get_parameter_value().double_value

        self._mode = mode.lower().strip()
        self._pub = None
        if self._mode == 'grid':
            self._pub = self.create_publisher(OccupancyGrid, self._grid_topic, 10)
            self.get_logger().info(
                f'Modo grid: publicando OccupancyGrid en {self._grid_topic} '
                f'({self._grid_cols} celdas, resolución {self._resolution} m).'
            )
        else:
            self.get_logger().info('Modo texto: distancia y estado en log.')

        self.create_subscription(Range, topic, self._on_range, 10)

    def _on_range(self, msg: Range) -> None:
        invalid = (not math.isfinite(msg.range)) or (msg.range < 0.0)
        valid = (not invalid) and (msg.range >= msg.min_range) and (msg.range <= msg.max_range)

        if self._mode != 'grid':
            if valid:
                self.get_logger().info(
                    f'distancia={msg.range:.3f} m  frame={msg.header.frame_id} '
                    f'[{msg.min_range:.2f}, {msg.max_range:.2f}]'
                )
            else:
                self.get_logger().warn('Lectura inválida o fuera de rango (p. ej. eco ausente).')
            return

        if self._pub is None:
            return

        cols = max(1, int(self._grid_cols))
        grid = OccupancyGrid()
        grid.header = msg.header
        grid.header.frame_id = msg.header.frame_id or 'ultrasonic_frame'
        grid.info = MapMetaData()
        grid.info.map_load_time = msg.header.stamp
        grid.info.resolution = float(self._resolution)
        grid.info.width = cols
        grid.info.height = 1
        grid.info.origin.position.x = 0.0
        grid.info.origin.position.y = 0.0
        grid.info.origin.position.z = 0.0
        grid.info.origin.orientation.w = 1.0

        data = [-1] * cols
        if valid:
            idx = int(msg.range / grid.info.resolution)
            idx = max(0, min(cols - 1, idx))
            data[idx] = 100
        grid.data = data
        self._pub.publish(grid)


def main() -> None:
    rclpy.init()
    node = RangeVisualizer()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()
